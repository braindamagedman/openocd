/***************************************************************************
 *   Copyright (C) 2008 by Spencer Oliver                                  *
 *   spen@spen-soft.co.uk                                                  *
 *                                                                         *
 *   Copyright (C) 2008 by David T.L. Wong                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mips32.h"
#include "jtag.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>

char* mips32_core_reg_list[] =
{
	"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
	"r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
	"r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
	"r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31",
	"status", "lo", "hi", "badvaddr", "cause", "pc"
};

mips32_core_reg_t mips32_core_reg_list_arch_info[MIPS32NUMCOREREGS] = 
{
	{0, NULL, NULL},
	{1, NULL, NULL},
	{2, NULL, NULL},
	{3, NULL, NULL},
	{4, NULL, NULL},
	{5, NULL, NULL},
	{6, NULL, NULL},
	{7, NULL, NULL},
	{8, NULL, NULL},
	{9, NULL, NULL},
	{10, NULL, NULL},
	{11, NULL, NULL},
	{12, NULL, NULL},
	{13, NULL, NULL},
	{14, NULL, NULL},
	{15, NULL, NULL},
	{16, NULL, NULL},
	{17, NULL, NULL},
	{18, NULL, NULL},
	{19, NULL, NULL},
	{20, NULL, NULL},
	{21, NULL, NULL},
	{22, NULL, NULL},
	{23, NULL, NULL},
	{24, NULL, NULL},
	{25, NULL, NULL},
	{26, NULL, NULL},
	{27, NULL, NULL},
	{28, NULL, NULL},
	{29, NULL, NULL},
	{30, NULL, NULL},
	{31, NULL, NULL},
	
	{32, NULL, NULL},
	{33, NULL, NULL},
	{34, NULL, NULL},
	{35, NULL, NULL},
	{36, NULL, NULL},
	{37, NULL, NULL},
};

u8 mips32_gdb_dummy_fsr_value[] = {0, 0, 0, 0};

reg_t mips32_gdb_dummy_fsr_reg =
{
	"GDB dummy floating-point status register", mips32_gdb_dummy_fsr_value, 0, 1, 32, NULL, 0, NULL, 0
};

u8 mips32_gdb_dummy_fir_value[] = {0, 0, 0, 0};

reg_t mips32_gdb_dummy_fir_reg =
{
	"GDB dummy floating-point register", mips32_gdb_dummy_fir_value, 0, 1, 32, NULL, 0, NULL, 0
};

int mips32_core_reg_arch_type = -1;

int mips32_get_core_reg(reg_t *reg)
{
	int retval;
	mips32_core_reg_t *mips32_reg = reg->arch_info;
	target_t *target = mips32_reg->target;
	mips32_common_t *mips32_target = target->arch_info;
	
	if (target->state != TARGET_HALTED)
	{
		return ERROR_TARGET_NOT_HALTED;
	}

	retval = mips32_target->read_core_reg(target, mips32_reg->num);
	
	return retval;
}

int mips32_set_core_reg(reg_t *reg, u8 *buf)
{
	mips32_core_reg_t *mips32_reg = reg->arch_info;
	target_t *target = mips32_reg->target;
	u32 value = buf_get_u32(buf, 0, 32);
		
	if (target->state != TARGET_HALTED)
	{
		return ERROR_TARGET_NOT_HALTED;
	}
		
	buf_set_u32(reg->value, 0, 32, value);
	reg->dirty = 1;
	reg->valid = 1;

	return ERROR_OK;
}

int mips32_read_core_reg(struct target_s *target, int num)
{
	u32 reg_value;
	mips32_core_reg_t *mips_core_reg;
	
	/* get pointers to arch-specific information */
	mips32_common_t *mips32 = target->arch_info;
		
	if ((num < 0) || (num >= MIPS32NUMCOREREGS))
		return ERROR_INVALID_ARGUMENTS;

	mips_core_reg = mips32->core_cache->reg_list[num].arch_info;
	reg_value = mips32->core_regs[num];
	buf_set_u32(mips32->core_cache->reg_list[num].value, 0, 32, reg_value);
	mips32->core_cache->reg_list[num].valid = 1;
	mips32->core_cache->reg_list[num].dirty = 0;
	
	return ERROR_OK;	
}

int mips32_write_core_reg(struct target_s *target, int num)
{
	u32 reg_value;
	mips32_core_reg_t *mips_core_reg;
	
	/* get pointers to arch-specific information */
	mips32_common_t *mips32 = target->arch_info;

	if ((num < 0) || (num >= MIPS32NUMCOREREGS))
		return ERROR_INVALID_ARGUMENTS;
	
	reg_value = buf_get_u32(mips32->core_cache->reg_list[num].value, 0, 32);
	mips_core_reg = mips32->core_cache->reg_list[num].arch_info;
	mips32->core_regs[num] = reg_value;
	LOG_DEBUG("write core reg %i value 0x%x", num , reg_value);
	mips32->core_cache->reg_list[num].valid = 1;
	mips32->core_cache->reg_list[num].dirty = 0;
	
	return ERROR_OK;
}

int mips32_invalidate_core_regs(target_t *target)
{
	/* get pointers to arch-specific information */
	mips32_common_t *mips32 = target->arch_info;
	int i;
	
	for (i = 0; i < mips32->core_cache->num_regs; i++)
	{
		mips32->core_cache->reg_list[i].valid = 0;
		mips32->core_cache->reg_list[i].dirty = 0;
	}
	
	return ERROR_OK;
}

int mips32_get_gdb_reg_list(target_t *target, reg_t **reg_list[], int *reg_list_size)
{
	/* get pointers to arch-specific information */
	mips32_common_t *mips32 = target->arch_info;
	int i;
	
	/* include fsr/fir reg */
	*reg_list_size = MIPS32NUMCOREREGS + 2;
	*reg_list = malloc(sizeof(reg_t*) * (*reg_list_size));
	
	for (i = 0; i < MIPS32NUMCOREREGS; i++)
	{
		(*reg_list)[i] = &mips32->core_cache->reg_list[i];
	}
	
	/* add dummy floating points regs */
	(*reg_list)[38] = &mips32_gdb_dummy_fsr_reg;
	(*reg_list)[39] = &mips32_gdb_dummy_fir_reg;
	
	return ERROR_OK;
}

int mips32_save_context(target_t *target)
{
	int i;
	
	/* get pointers to arch-specific information */
	mips32_common_t *mips32 = target->arch_info;
	mips_ejtag_t *ejtag_info = &mips32->ejtag_info;
	
	/* read core registers */
	mips32_pracc_read_regs(ejtag_info, mips32->core_regs);
	
	for (i = 0; i < MIPS32NUMCOREREGS; i++)
	{
		if (!mips32->core_cache->reg_list[i].valid)
		{
			mips32->read_core_reg(target, i);
		}
	}
	
	return ERROR_OK;		
}

int mips32_restore_context(target_t *target)
{
	int i;
	
	/* get pointers to arch-specific information */
	mips32_common_t *mips32 = target->arch_info;
	mips_ejtag_t *ejtag_info = &mips32->ejtag_info;
	
	for (i = 0; i < MIPS32NUMCOREREGS; i++)
	{
		if (mips32->core_cache->reg_list[i].dirty)
		{
			mips32->write_core_reg(target, i);
		}
	}
	
	/* write core regs */
	mips32_pracc_write_regs(ejtag_info, mips32->core_regs);
	
	return ERROR_OK;		
}

int mips32_arch_state(struct target_s *target)
{
	mips32_common_t *mips32 = target->arch_info;
	
	if (mips32->common_magic != MIPS32_COMMON_MAGIC)
	{
		LOG_ERROR("BUG: called for a non-MIPS32 target");
		exit(-1);
	}
	
	LOG_USER("target halted due to %s, pc: 0x%8.8x",
		Jim_Nvp_value2name_simple( nvp_target_debug_reason, target->debug_reason )->name ,
		buf_get_u32(mips32->core_cache->reg_list[MIPS32_PC].value, 0, 32));
	
	return ERROR_OK;
}

reg_cache_t *mips32_build_reg_cache(target_t *target)
{
	/* get pointers to arch-specific information */
	mips32_common_t *mips32 = target->arch_info;

	int num_regs = MIPS32NUMCOREREGS;
	reg_cache_t **cache_p = register_get_last_cache_p(&target->reg_cache);
	reg_cache_t *cache = malloc(sizeof(reg_cache_t));
	reg_t *reg_list = malloc(sizeof(reg_t) * num_regs);
	mips32_core_reg_t *arch_info = malloc(sizeof(mips32_core_reg_t) * num_regs);
	int i;
	
	if (mips32_core_reg_arch_type == -1)
		mips32_core_reg_arch_type = register_reg_arch_type(mips32_get_core_reg, mips32_set_core_reg);
		
	/* Build the process context cache */ 
	cache->name = "mips32 registers";
	cache->next = NULL;
	cache->reg_list = reg_list;
	cache->num_regs = num_regs;
	(*cache_p) = cache;
	mips32->core_cache = cache;
	
	for (i = 0; i < num_regs; i++)
	{
		arch_info[i] = mips32_core_reg_list_arch_info[i];
		arch_info[i].target = target;
		arch_info[i].mips32_common = mips32;
		reg_list[i].name = mips32_core_reg_list[i];
		reg_list[i].size = 32;
		reg_list[i].value = calloc(1, 4);
		reg_list[i].dirty = 0;
		reg_list[i].valid = 0;
		reg_list[i].bitfield_desc = NULL;
		reg_list[i].num_bitfields = 0;
		reg_list[i].arch_type = mips32_core_reg_arch_type;
		reg_list[i].arch_info = &arch_info[i];
	}
	
	return cache;
}

int mips32_init_arch_info(target_t *target, mips32_common_t *mips32, int chain_pos, const char *variant)
{
	target->arch_info = mips32;
	mips32->common_magic = MIPS32_COMMON_MAGIC;
	
	mips32->ejtag_info.chain_pos = chain_pos;
	mips32->read_core_reg = mips32_read_core_reg;
	mips32->write_core_reg = mips32_write_core_reg;
	
	return ERROR_OK;
}

int mips32_register_commands(struct command_context_s *cmd_ctx)
{
	return ERROR_OK;
}

int mips32_run_algorithm(struct target_s *target, int num_mem_params, mem_param_t *mem_params, int num_reg_params, reg_param_t *reg_params, u32 entry_point, u32 exit_point, int timeout_ms, void *arch_info)
{
	/*TODO*/
	return ERROR_OK;
}
