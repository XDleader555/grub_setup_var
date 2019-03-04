/* setup_var.c - InsydeH2o Setup variable modification tool, can modify single
 *               bytes within the Setup variable */
/*  (c) 2009 by Bernhard Froemel
 *
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2002,2003,2005,2006,2007,2008,2009,2008  Free Software Foundation, Inc.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <grub/types.h>
#include <grub/dl.h>
#include <grub/misc.h>
#include <grub/command.h>
#include <grub/file.h>
#include <grub/efi/efi.h>
#include <grub/pci.h>

#define MAX_VARIABLE_SIZE		(1024)
#define MAX_VAR_DATA_SIZE		(65536)

#define CMDNAME_SETUP_VAR ("setup_var")
#define CMDCHECK_SETUP_VAR (8)

GRUB_MOD_LICENSE("GPLv3+");

const char* efichartochar(char* destination, grub_efi_char16_t* source);

const char* efichartochar(char* destination, grub_efi_char16_t* source) {
	int size;
	grub_efi_char16_t* sptr;
	char* dptr;

	sptr = source;
	dptr = destination;

	/* Get string size */
	size = 0;
	while(*sptr++ != 0x0)
		size ++;

	/* Reset pointer */
	sptr = source;

	/* Write data */
	while(*sptr != 0x0)
		*(dptr++) = (grub_uint8_t) *(sptr++);

	/* Terminate string */
	*dptr = '\0';

	return destination;
}

static grub_err_t grub_cmd_setup_var (grub_command_t cmd, int argc, char *argv[])
{
	grub_efi_status_t	status;
	grub_efi_guid_t		guid;
	grub_uint8_t		tmp_data[MAX_VAR_DATA_SIZE];
	grub_uint16_t		offset;
	grub_efi_uintn_t	setup_var_size;
	grub_uint8_t		set_value;
	grub_efi_uint32_t	setup_var_attr = 0x7;
	char* endptr;

	grub_efi_char16_t	name[MAX_VARIABLE_SIZE/2];
	grub_efi_uintn_t	name_size;
	char			namestr[MAX_VARIABLE_SIZE/2];
	
	/* No arguments inputted */
	if (argc == 0) {
		grub_printf("\n\n(c) 2009 by Bernhard Froemel <bfroemel@gmail.com> testv4\n");
	}

	if(argc == 0 || argc > 3)
		return grub_error(GRUB_ERR_BAD_ARGUMENT, "Usage: %s storename offset [setval]", cmd->name);

	name[0] = 0x0;

	/* Search for user specified setup variable */
	if(argc >= 1) {
		grub_printf("Searching for variable store \"%s\"...", argv[0]);
		do {
			name_size = MAX_VARIABLE_SIZE;
			status = efi_call_3(grub_efi_system_table->runtime_services->get_next_variable_name,
									&name_size,
									name,
									&guid
								);

			
			/* Finished traversing VSS */
			if(status == GRUB_EFI_NOT_FOUND) {
				grub_printf("Unable to find variable store.\n");
				break;
			}
			
			/* Other error */
			if(status)
				grub_printf("Error status: 0x%02x\n", (grub_uint32_t) status);

			/* Next variable Exists */
			if(!status) {

				/* Check if it's the user's variable store */
				if(grub_strcmp(efichartochar(namestr, name), argv[0]) == 0) {
					grub_printf("store name: %s, store size: %u, store guid: %08x-%04x-%04x - %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\n\n",
									efichartochar(namestr, name),
									(grub_uint32_t) name_size,
									guid.data1,
									guid.data2,
									guid.data3,
									guid.data4[0], guid.data4[1], guid.data4[2], guid.data4[3], guid.data4[4], guid.data4[5], guid.data4[6], guid.data4[7]
								);

					if(argc >= 2 && argc <= 3) {
						/* Get variable offset */
						grub_errno = 0;
						offset = grub_strtoul(argv[1], &endptr, 16);

						if(endptr == argv[1] || grub_errno != 0)
							return grub_error(GRUB_ERR_BAD_ARGUMENT, "Invalid offset arguent. Please provide a hex value (e.g. 0x1af).");

						/* Get store data */
						status = efi_call_5(grub_efi_system_table->runtime_services->get_variable, 
												name,
												&guid,
												&setup_var_attr,
												&setup_var_size,
												tmp_data
											);

						/* Force buffer size */
						if(status == GRUB_EFI_BUFFER_TOO_SMALL) {
							grub_printf("Expected a different size of the Setup variable (got %d (0x%x) bytes). Continue with care...\n", (int)setup_var_size, (int)setup_var_size);
							status = efi_call_5(grub_efi_system_table->runtime_services->get_variable, 
													name,
													&guid,
													&setup_var_attr,
													&setup_var_size,
													tmp_data
												);
						}

						if(status)
							return grub_error(GRUB_ERR_INVALID_COMMAND, "Unable to open variable store! (Error: 0x%016x)", status);
						
						//status = GRUB_EFI_SUCCESS;

						grub_printf("Successfully opened \"%s\" from VSS (got %d (0x%x) bytes).\n", efichartochar(namestr, name), (int)setup_var_size, (int)setup_var_size);

						if(offset > setup_var_size)
							return grub_error(GRUB_ERR_BAD_ARGUMENT, "Offset out of range. Did you open the right variable store?");

						grub_printf("Offset 0x%02x is: 0x%02x\n", offset, tmp_data[offset]);


						/* Write Variable if there is user input */
						if(argc == 3) {
							set_value = grub_strtoul(argv[2], &endptr, 16);

							if(endptr == argv[2] || grub_errno != 0) 
								return grub_error(GRUB_ERR_BAD_ARGUMENT, "Invalid value argument. Please provide a hex value (e.g. 0x01).");

							grub_printf("Writing 0x%02x to offset 0x%02x\n", set_value, offset);
							tmp_data[offset] = set_value;

							/* Write modified data to the variable store */
							status = efi_call_5(grub_efi_system_table->runtime_services->set_variable,
													name,
													&guid,
													setup_var_attr,
													setup_var_size,
													tmp_data
												);

							if(status)
								return grub_error(GRUB_ERR_INVALID_COMMAND, "Unable to write data! (Error: 0x%016x)", status);
						}

					}

					/* Return early because we're done */
					return grub_errno;
				}
			}
		} while (!status);
	}
	return grub_errno;
}

static grub_err_t grub_cmd_lsefivar (grub_command_t cmd __attribute__ ((unused)),
		   int argc __attribute__ ((unused)), char *argv[] __attribute__ ((unused)))
{
	grub_efi_status_t	status;
	grub_efi_guid_t		guid;
	grub_efi_uint32_t	setup_var_attr;
	grub_efi_uintn_t	setup_var_size;
	grub_uint8_t		tmp_data[MAX_VAR_DATA_SIZE];

	grub_efi_char16_t	name[MAX_VARIABLE_SIZE/2];
	grub_efi_uintn_t	name_size;
	char			namestr[MAX_VARIABLE_SIZE/2];

	name[0] = 0x0;

	// TODO: List only efi stores

	/* scan for Setup variable */
	grub_printf("Listing EFI variables...\n");
	do {
		name_size = MAX_VARIABLE_SIZE;
		status = efi_call_3(grub_efi_system_table->runtime_services->get_next_variable_name,
								&name_size,
								name,
								&guid
							);

		
		/* Finished traversing VSS */
		if(status == GRUB_EFI_NOT_FOUND)
			break;
		
		/* Other error */
		if(status)
			grub_printf("status: 0x%02x\n", (grub_uint32_t) status);

		/* Variable Exists, print data */
		if(!status) {
			setup_var_size = 1;
			status = efi_call_5(grub_efi_system_table->runtime_services->get_variable, 
									name,
									&guid,
									&setup_var_attr,
									&setup_var_size,
									tmp_data
								);

			/* Skip if we're unable to read the variable */
			if (status && status != GRUB_EFI_BUFFER_TOO_SMALL) {
			    grub_printf("get_variable exited with code 0x%x\n", (grub_uint32_t)status);
			    continue;
			}

			/* Clear error */
			status = GRUB_EFI_SUCCESS;

			/* Print data */
			grub_printf("name size: %02u, var size: %06u (0x%06x), var guid: %08x-%04x-%04x - %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x, name: %s\n",
							(grub_uint32_t) name_size,
							(grub_uint32_t) setup_var_size,
							(grub_uint32_t) setup_var_size,
							guid.data1,
							guid.data2,
							guid.data3,
							guid.data4[0], guid.data4[1], guid.data4[2], guid.data4[3], guid.data4[4], guid.data4[5], guid.data4[6], guid.data4[7],
							efichartochar(namestr, name)
						);
		}
	} while (!status);

	return grub_errno;
}

static grub_command_t cmd_setup_var;
static grub_command_t cmd_setup_lsvar;

GRUB_MOD_INIT(setup_var)
{
	cmd_setup_var = grub_register_command ("setup_var", grub_cmd_setup_var,
							"setup_var storename offset [setval]",
							"Read/Write specific (byte) offset of setup variable."
						);

	cmd_setup_lsvar = grub_register_command ("lsefivar", grub_cmd_lsefivar,
							"lsefivar",
							"Lists all efi variables."
						);
}

GRUB_MOD_FINI(setup_var)
{
	grub_unregister_command(cmd_setup_var);
	grub_unregister_command(cmd_setup_lsvar);
}
