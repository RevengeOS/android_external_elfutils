/* Copyright (C) 2005 Red Hat, Inc.
   This file is part of Red Hat elfutils.

   Red Hat elfutils is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by the
   Free Software Foundation; version 2 of the License.

   Red Hat elfutils is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with Red Hat elfutils; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Red Hat elfutils is an included package of the Open Invention Network.
   An included package of the Open Invention Network is a package for which
   Open Invention Network licensees cross-license their patents.  No patent
   license is granted, either expressly or impliedly, by designation as an
   included package.  Should you wish to participate in the Open Invention
   Network licensing program, please visit www.openinventionnetwork.com
   <http://www.openinventionnetwork.com>.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <locale.h>
#include <argp.h>
#include <assert.h>
#include ELFUTILS_HEADER(dwfl)


static int
first_module (Dwfl_Module *mod,
	      void **userdatap __attribute__ ((unused)),
	      const char *name __attribute__ ((unused)),
	      Dwarf_Addr low_addr __attribute__ ((unused)),
	      void *arg)
{
  Dwarf_Addr bias;
  if (dwfl_module_getelf (mod, &bias) == NULL) /* Not really a module.  */
    return DWARF_CB_OK;

  *(Dwfl_Module **) arg = mod;
  return DWARF_CB_ABORT;
}


struct state
{
  struct reginfo *info;
  int nregs;
};

struct reginfo
{
  const char *set, *pfx;
  int regno;
  char name[8];
};

static int
compare (const void *r1, const void *r2)
{
  const struct reginfo *a = r1, *b = r2;
  if (a->set == b->set)
    return a->regno - b->regno;
  if (a->set == NULL)
    return 1;
  if (b->set == NULL)
    return -1;
  if (!strcmp (a->set, "integer"))
    return -1;
  if (!strcmp (b->set, "integer"))
    return 1;
  return strcmp (a->set, b->set);
}

static int
one_register (void *arg,
	      int regno,
	      const char *setname,
	      const char *prefix,
	      const char *regname)
{
  struct state *state = arg;

  if (regno >= state->nregs)
    {
      state->info = realloc (state->info, (regno + 1) * sizeof state->info[0]);
      memset (&state->info[state->nregs], 0,
	      ((void *) &state->info[regno + 1]
	       - (void *) &state->info[state->nregs]));
      state->nregs = regno + 1;
    }

  state->info[regno].regno = regno;
  state->info[regno].set = setname;
  state->info[regno].pfx = prefix;
  strcpy (state->info[regno].name, regname);

  return DWARF_CB_OK;
}


static int
match_register (void *arg,
		int regno,
		const char *setname,
		const char *prefix,
		const char *regname)
{
  if (regno == *(int *) arg)
    printf ("%5d => %s register %s%s\n", regno, setname, prefix, regname);

  return DWARF_CB_ABORT;
}


int
main (int argc, char **argv)
{
  int remaining;

  /* Set locale.  */
  (void) setlocale (LC_ALL, "");

  Dwfl *dwfl = NULL;
  (void) argp_parse (dwfl_standard_argp (), argc, argv, 0, &remaining, &dwfl);
  assert (dwfl != NULL);

  Dwfl_Module *mod = NULL;
  if (dwfl_getmodules (dwfl, &first_module, &mod, 0) < 0)
    error (EXIT_FAILURE, 0, "dwfl_getmodules: %s", dwfl_errmsg (-1));

  if (remaining == argc)
    {
      struct state state = { NULL, 0 };
      int result = dwfl_module_register_names (mod, &one_register, &state);
      if (result != 0 || state.nregs == 0)
	error (EXIT_FAILURE, 0, "dwfl_module_register_names: %s",
	       result ? dwfl_errmsg (-1) : "no backend registers known");

      qsort (state.info, state.nregs, sizeof state.info[0], &compare);

      const char *set = NULL;
      for (int i = 0; i < state.nregs; ++i)
	if (state.info[i].set != NULL)
	  {
	    if (set != state.info[i].set)
	      printf ("%s registers:\n", state.info[i].set);
	    set = state.info[i].set;

	    printf ("\t%3d: %s%s (%s)\n", state.info[i].regno,
		    state.info[i].pfx ?: "", state.info[i].name,
		    state.info[i].name);
	  }
    }
  else
    do
      {
	const char *arg = argv[remaining++];
	int regno = atoi (arg);
	int result = dwfl_module_register_names (mod, &match_register, &regno);
	if (result != DWARF_CB_ABORT)
	  error (EXIT_FAILURE, 0, "dwfl_module_register_names: %s",
		 result ? dwfl_errmsg (-1) : "no backend registers known");
      }
    while (remaining < argc);

  dwfl_end (dwfl);

  return 0;
}
