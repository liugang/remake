/* $Id: dbg_break.c,v 1.2 2005/12/06 04:59:06 rockyb Exp $
Copyright (C) 2005 rocky@panix.com
This file is part of GNU Make.

GNU Make is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Make is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Make; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/** debugger command stack routines. */

#include <assert.h>
#include "dbg_break.h"
#include "make.h"
#include "file.h"

/*! Node for an item in the target call stack */
struct breakpoint_node
{
  file_t            *p_target;
  breakpoint_node_t *p_next;
};

/** Pointers to top/bottom of current breakpoint target stack */
breakpoint_node_t *p_breakpoint_top    = NULL;
breakpoint_node_t *p_breakpoint_bottom = NULL;

unsigned int i_breakpoints = 0;

/*! Add "p_target" to the list of breakpoints. Return true if 
    there were no errors
*/
bool 
add_breakpoint (file_t *p_target) 
{
  breakpoint_node_t *p_new   = CALLOC (breakpoint_node_t, 1);

  if (!p_new) return false;

  /* Add breakpoint to list of breakpoints. */
  if (!i_breakpoints) {
    assert(!p_breakpoint_top && !p_breakpoint_bottom);
    p_breakpoint_top            = p_breakpoint_bottom = p_new;
  } else {
    p_breakpoint_bottom->p_next = p_new;
  }
  p_breakpoint_bottom           = p_new;
  p_new->p_target               = p_target;
  i_breakpoints++;


  /* Finally, note that we are tracing this target. */
  if (p_target->tracing) {
    printf("Breakpoint already set at target %s; nothing done.\n", 
	   p_target->name);
  } else {
    p_target->tracing = 1;
    printf("Breakpoint on target %s set.\n", p_target->name);
  }
  return true;
  
};

/*! Remove breakpoint i from the list of breakpoints. Return true if 
    there were no errors
*/
bool 
remove_breakpoint (unsigned int i) 
{
  if (!i) {
    printf("Invalid Breakpoint number 0.\n");
    return false;
  }
  if (i > i_breakpoints) {
    printf("Breakpoint number %d is too high. " 
	   "Only %d breakpoints have been set.\n", i, i_breakpoints);
    return false;
  } else {
    /* Find breakpoint i */
    breakpoint_node_t *p_prev = NULL;
    breakpoint_node_t *p;
    unsigned int j=1;
    for (p = p_breakpoint_top; p && i>j; p = p->p_next) {
      j++;
      p_prev = p;
    }

    if (p && i==j) {
      /* Delete breakpoint */
      if (!p->p_next) p_breakpoint_bottom = p_prev;
      if ( (p == p_breakpoint_top) ) p_breakpoint_top = p->p_next;

      if (p_prev) p_prev->p_next = p->p_next;

      i_breakpoints--;

      if (p->p_target->tracing) {
	p->p_target->tracing = 0;
	printf("Breakpoint on target %s cleared\n", p->p_target->name);
      } else {
	printf("No breakpoint at target %s; nothing cleared.\n", 
	       p->p_target->name);
      }

      /* Free resources associated with breakpoint. */
      free(p);

    } else {
      printf("Internal inconsistency - "
	     "we should have found breakpoint %d but didn't\n", i);
      return false;
    }
  }
  ;
}

/*! List breakpoints.*/
void
list_breakpoints (void) 
{
  breakpoint_node_t *p;
  unsigned int i=1;

  if (!i_breakpoints) {
    printf("No breakpoints.\n");
    return;
  }

  printf(  "Num Type           Disp Enb target     What\n");
  for (p = p_breakpoint_top; p; p = p->p_next) {
    printf("%3d breakpoint     keep y   in %s at ", i,
	   p->p_target->name);
    print_floc_prefix(&(p->p_target->floc));
    printf("\n");
    i++;
  }
}
 