/**
 *  Copyright 2020 by Zeroual Aymene <ayemenzeroual@gmail.com>
 *
 *  Licensed under GNU General Public License 3.0 or later.
 *  Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 *
 * FILE: class_huf.cpp
 *
 * DECRP:
 *	- Functions difintions
 *  - tiny error handling system
 *
 */

#include "class_huf.h"

void HUF_Bank::M_DisplayError()
{
	printf("HUF Bank %s : %s", this->file_name, HUF_GetError());
}

bool HUF_Bank::Init(const char* file_name)
{

	this->file_name = file_name;
	this->chunks_size = 0;
	
	if (!HUF_LoadHeader(file_name, &this->m_header, nullptr))
	{
		this->M_DisplayError();
		return false;
	}

	if (!HUF_LoadDirsTable(this->file_name, this->m_dirs_table, &this->chunks_size))
	{
		this->M_DisplayError();
		return false;
	}

	printf("HUF Bankk %s : Loaded !\n");

	return true;
}

bool HUF_Bank::Add(
					const char* name,
					char*		buffer,
					int			buffer_size
				  )
{
	if (!HUF_AddChunk(
			this->file_name, this->m_header,
			name           , buffer,
			buffer_size    , this->m_dirs_table
	))
	{
		this->M_DisplayError();
		return false;
	}

	return true;
}

bool HUF_Bank::Get(
					const char* dir_name,
					char* out_buffer,
					int* out_buffer_size
				  )
{
	if (!HUF_LoadChunk(
			this->file_name, this->m_header,
			dir_name       , this->m_dirs_table,
			out_buffer     , out_buffer_size
	))
	{
		this->M_DisplayError();
		return false;
	}

	return true;
}

bool HUF_Bank::Remove(const char* desired_dir_name)
{

	if (!HUF_DeleteChunk(
		this->file_name , this->m_header,
		desired_dir_name, this->m_dirs_table
	))
	{
		this->M_DisplayError();
		return false;
	}

	return true;
}

void HUF_Bank::Destory()
{
	if (this->m_header != nullptr)
		delete this->m_header;

	if (this->m_dirs_table != nullptr)
		delete[] this->m_dirs_table;
}
