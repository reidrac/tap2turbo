/*
tap2turbo
Copyright (c) 2015 Juan J. Martinez <jjm@usebox.net>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>

#include <libspectrum.h>

#define VERSION "1.0"

void
fatal(char *argv0, char *message)
{
	fprintf(stderr, "%s: %s\n", argv0, message);
	exit(1);
}

void
help(char *argv0)
{
	fprintf(stderr,"Convert a tape file into .tzx setting turbo blocks\n"
			       "Copyright (C) 2015 Juan J. Martinez <jjm@usebox.net>\n\n"
			       "Usage: %s [-h] [-q] [-o output] input [block [block [...]]]\n\n"
				   "   input        input filename\n"
				   "   block        block number to set to turbo mode (0 to n).\n\n"
				   "   -h           this help screen\n"
				   "   -q           quiet\n"
				   "   -v           print version an exit\n"
				   "   -t params    turbo parameters (ppulse:plen:sync1p:sync2p:zerop:onep)\n"
				   "   -o output    output filename (default: output.tzx)\n\n"
				   "Turbo Parameters:\n\n"
				   "    ppulse: pilot pulse (default: 2168)\n"
				   "      plen: pilot length (default: 3323)\n"
				   "    sync1p: sync 1 pulse (default: 667)\n"
				   "    sync2p: sync 2 pulse (default: 735)\n"
				   "     zerop: zero pulse (default: 855)\n"
				   "      onep: one pulse (default: 1710)\n\n"
				   "Further info: http://www.usebox.net/jjm/zxdev/\n\n"
				   , argv0);
}

int
main(int argc, char *argv[])
{
	int opt, i, quiet = 0;
	char *output = "output.tzx", *p;
	int turbo_params[6] = { 2168, 3323, 667, 735, 855, 1710 };

	FILE *fd;
	size_t data_len;

	libspectrum_tape *tape;
	libspectrum_byte *data;
	libspectrum_tape_block *block;
	libspectrum_dword pause, pause_tstates;

	while ((opt = getopt(argc, argv, "vhqt:o:")) != -1)
	{
		switch(opt)
		{
			case 'q':
				quiet = 1;
				break;
			case 't':
				if (sscanf(optarg, "%i:%i:%i:%i:%i:%i", &turbo_params[0],
					&turbo_params[1], &turbo_params[2], &turbo_params[3], 
					&turbo_params[4], &turbo_params[5]) != 6)
					fatal(argv[0], "Failed to parse turbo params");
				break;
			case 'o':
				output = strdup(optarg);
				break;
			case 'h':
				help(argv[0]);
				exit(0);
			case 'v':
				fprintf(stderr,  VERSION "\n");
				exit(0);
			default:
				fprintf(stderr, "\n");
				help(argv[0]);
				exit(1);
		}
	}

	if (optind >= argc)
		fatal(argv[0], "Expected input filename to process");

	if (libspectrum_init())
		fatal(argv[0], "Failed to init libspectrum");

	fd = fopen(argv[optind], "rb");
	if (!fd)
		fatal(argv[0], "Failed to open input file");

	fseek(fd, 0, SEEK_END);
	data_len = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	data = (libspectrum_byte *) malloc(data_len);
	if (!data)
	{
		fclose(fd);
		fatal(argv[0], "Failed to allocate memory");
	}

	if (fread(data, sizeof(libspectrum_byte), data_len, fd) != data_len)
	{
		fclose(fd);
		free(data);
		fatal(argv[0], "Failed to read input file");
	}
	fclose(fd);

	tape = libspectrum_tape_alloc();
	if (!tape)
	{
		fclose(fd);
		free(data);
		fatal(argv[0], "Failed to allocate tape memory");
	}

	if (libspectrum_tape_read(tape, data, data_len, LIBSPECTRUM_ID_UNKNOWN, argv[optind]))
	{
		fclose(fd);
		free(data);
		free(tape);
		fatal(argv[0], "Failed to read tape");
	}
	free(data);

	for (i = optind + 1; i < argc; i++)
	{
		for (p = argv[i]; *p && isdigit(*p); p++);

		if (*p)
		{
			fprintf(stderr, "WARNING: Parameter is not a valid block number, skipping\n");
			continue;
		}

		if (libspectrum_tape_nth_block(tape, atoi(argv[i])))
			continue;

		block = libspectrum_tape_current_block(tape);
		if (!block)
			continue;

		if (!quiet)
			printf("%s: Setting block #%i (type 0x%02x): ", argv[0], atoi(argv[i]),
					libspectrum_tape_block_type(block));

		data = libspectrum_tape_block_data(block);
		data_len = libspectrum_tape_block_data_length(block);
		pause = libspectrum_tape_block_pause(block);
		pause_tstates = libspectrum_tape_block_pause_tstates(block);

		if (libspectrum_tape_block_set_type(block, LIBSPECTRUM_TAPE_BLOCK_TURBO)
			|| libspectrum_tape_block_set_pilot_pulses(block, turbo_params[1])
			|| libspectrum_tape_block_set_pilot_length(block, turbo_params[0])
			|| libspectrum_tape_block_set_sync1_length(block, turbo_params[2])
			|| libspectrum_tape_block_set_sync2_length(block, turbo_params[3])
			|| libspectrum_tape_block_set_bit0_length(block, turbo_params[4])
			|| libspectrum_tape_block_set_bit1_length(block, turbo_params[5])
			|| libspectrum_tape_block_set_data(block, data)
		 	|| libspectrum_tape_block_set_data_length(block, data_len)
			|| libspectrum_tape_block_set_pause(block, pause)
			|| libspectrum_tape_block_set_pause_tstates(block, pause_tstates)
			|| libspectrum_tape_block_set_bits_in_last_byte(block, 8)
			)
		{
			if (!quiet)
				printf("failed!\n");
			fprintf(stderr, "WARNING: Failed to set block %i\n", atoi(argv[i]));
			continue;
		}

		if (!quiet)
			printf("OK\n");
	}

	data = NULL;
	data_len = 0;
	if (libspectrum_tape_write(&data, &data_len, tape, LIBSPECTRUM_ID_TAPE_TZX))
	{
		free(tape);
		fatal(argv[0], "Failed to write the output file");
	}

	fd = fopen(output, "wb");
	if (!fd)
	{
		free(data);
		free(tape);
		fatal(argv[0], "Failed to open output file");
	}

	if (fwrite(data, sizeof(libspectrum_byte), data_len, fd) != data_len)
	{
		fclose(fd);
		free(data);
		free(tape);
		fatal(argv[0], "Failed to write output file");
	}
	fclose(fd);
	free(data);
	free(tape);

	exit(0);
}

