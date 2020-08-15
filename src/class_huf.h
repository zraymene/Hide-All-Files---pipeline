/**
 *  Copyright 2020 by Zeroual Aymene <ayemenzeroual@gmail.com>
 *
 *  Licensed under GNU General Public License 3.0 or later.
 *  Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 *
 * FILE: class_huf.h
 *
 * DECRP:
 *  - Class Example/Template to show how to use HUF Core 

 *  
 */

#pragma once

#include "haf_util.h"

class HUF_Bank
{
	public:

		HUF_Bank () {};
		~HUF_Bank() {};

		bool Init( const char* file_name );
		// Add data to The bank
		// Return true on success , false on failure
		bool Add(
					const char* name,
					char* buffer,
					uint32_t			buffer_size
				);
		// Get data from The bank
		// Return true on success , false on failure
		// The buffer and the buffer size as an args
		bool Get( 
					const char* dir_name,
					char*       out_buffer,
					uint32_t*     out_buffer_size
				);

		// Remove data from the file
		bool Remove( const char* desired_dir_name );

		void PrintDirectories();

		// Clean up every thing 
		void Destory();

	private:

		void M_DisplayError();

		const char* file_name        = nullptr;

		// Holds bank header
		header_t*		m_header     = nullptr;			
		// Directories table
		directory_t*	m_dirs_table = nullptr;		

		// Size of all chunks in bytes
		uint32_t			chunks_size  = 0;		

};