#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <gelf.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


#define PAGESIZE 4096


int
main (int argc, char *argv[])
{
  int cnt;
  int verbose = 0;

  elf_version (EV_CURRENT);

  for (cnt = 1; cnt < argc; ++cnt)
    {
      int fd;
      Elf *elf;
      int writable = 0;
      int cow = 0;
      int readonly = 0;
      int relro = 0;
      int inner;

      if (strcmp (argv[cnt], "--verbose") == 0)
	{
	  verbose = 1;
	  continue;
	}

      fd = open (argv[cnt], O_RDONLY);
      if (fd == -1)
	{
	  error (0, errno, "cannot open %s", argv[cnt]);
	  continue;
	}

      elf = elf_begin (fd, ELF_C_READ, NULL);
      if (elf == NULL)
	{
	  error (0, 0, "cannot get ELF handle: %s", elf_errmsg (-1));
	  close (fd);
	  continue;
	}

      GElf_Ehdr ehdr_mem;
      GElf_Ehdr *ehdr = gelf_getehdr (elf, &ehdr_mem);
      if (ehdr == NULL)
	{
	  error (0, 0, "cannot get ELF header: %s", elf_errmsg (-1));
	  close (fd);
	  continue;
	}

      for (inner = 0; inner < ehdr->e_phnum; ++inner)
	{
	  GElf_Phdr phdr_mem;
	  GElf_Phdr *phdr = gelf_getphdr (elf, inner, &phdr_mem);
	  if (phdr == NULL)
	    {
	      error (0, 0, "cannot get program header: %s", elf_errmsg (-1));
	      close (fd);
	      continue;
	    }

	  if (phdr->p_type == PT_LOAD)
	    {
	      uint64_t start = phdr->p_offset & ~(PAGESIZE - 1);
	      uint64_t fileend = phdr->p_offset + phdr->p_filesz;
	      uint64_t end = phdr->p_offset + phdr->p_memsz;

	      if (verbose)
		printf ("  segment %d: +%" PRIu64 " bytes\n",
			inner, end % PAGESIZE);

	      if (phdr->p_flags & PF_W)
		{
		  writable += (end - start + PAGESIZE - 1) / PAGESIZE;
		  cow += (fileend - start + PAGESIZE - 1) / PAGESIZE;
		}
	      else
		readonly += (end - start + PAGESIZE - 1) / PAGESIZE;
	    }
	  else if (phdr->p_type == PT_GNU_RELRO)
	    {
	      uint64_t start = phdr->p_offset & ~(PAGESIZE - 1);
	      uint64_t end = phdr->p_offset + phdr->p_memsz;

	      relro += (end - start + PAGESIZE - 1) / PAGESIZE;
	    }
	}

      elf_end (elf);

      printf ("%30s: %4d read-only, %4d writable, %5d COW, %5d relro\n",
	      argv[cnt], readonly, writable, cow, relro);
    }

  return 0;
}
