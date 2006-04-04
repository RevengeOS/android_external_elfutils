/* Return segment type name.
   Copyright (C) 2001, 2002, 2004 Red Hat, Inc.
   This file is part of Red Hat elfutils.
   Written by Ulrich Drepper <drepper@redhat.com>, 2001.

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

   In addition, as a special exception, Red Hat, Inc. gives You the
   additional right to link the code of Red Hat elfutils with code licensed
   under any Open Source Initiative certified open source license
   (http://www.opensource.org/licenses/index.php) which requires the
   distribution of source code with any binary distribution and to
   distribute linked combinations of the two.  Non-GPL Code permitted under
   this exception must only link to the code of Red Hat elfutils through
   those well defined interfaces identified in the file named EXCEPTION
   found in the source code files (the "Approved Interfaces").  The files
   of Non-GPL Code may instantiate templates or use macros or inline
   functions from the Approved Interfaces without causing the resulting
   work to be covered by the GNU General Public License.  Only Red Hat,
   Inc. may make changes or additions to the list of Approved Interfaces.
   Red Hat's grant of this exception is conditioned upon your not adding
   any new exceptions.  If you wish to add a new Approved Interface or
   exception, please contact Red Hat.  You must obey the GNU General Public
   License in all respects for all of the Red Hat elfutils code and other
   code used in conjunction with Red Hat elfutils except the Non-GPL Code
   covered by this exception.  If you modify this file, you may extend this
   exception to your version of the file, but you are not obligated to do
   so.  If you do not wish to provide this exception without modification,
   you must delete this exception statement from your version and license
   this file solely under the GPL without exception.

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
#include <libeblP.h>


const char *
ebl_segment_type_name (ebl, segment, buf, len)
     Ebl *ebl;
     int segment;
     char *buf;
     size_t len;
{
  const char *res;

  res = ebl != NULL ? ebl->segment_type_name (segment, buf, len) : NULL;
  if (res == NULL)
    {
      static const char *ptypes[PT_NUM] =
	{
#define PTYPE(name) [PT_##name] = #name
	  PTYPE (NULL),
	  PTYPE (LOAD),
	  PTYPE (DYNAMIC),
	  PTYPE (INTERP),
	  PTYPE (NOTE),
	  PTYPE (SHLIB),
	  PTYPE (PHDR),
	  PTYPE (TLS)
	};

      /* Is it one of the standard segment types?  */
      if (segment >= PT_NULL && segment < PT_NUM)
	res = ptypes[segment];
      else if (segment == PT_GNU_EH_FRAME)
	res = "GNU_EH_FRAME";
      else if (segment == PT_GNU_STACK)
	res = "GNU_STACK";
      else if (segment == PT_GNU_RELRO)
	res = "GNU_RELRO";
      else if (segment == PT_SUNWBSS)
	res = "SUNWBSS";
      else if (segment == PT_SUNWSTACK)
	res = "SUNWSTACK";
      else
	{
	  if (segment >= PT_LOOS && segment <= PT_HIOS)
	    snprintf (buf, len, "LOOS+%d", segment - PT_LOOS);
	  else if (segment >= PT_LOPROC && segment <= PT_HIPROC)
	    snprintf (buf, len, "LOPROC+%d", segment - PT_LOPROC);
	  else
	    snprintf (buf, len, "%s: %d", gettext ("<unknown>"), segment);

	  res = buf;
	}
    }

  return res;
}
