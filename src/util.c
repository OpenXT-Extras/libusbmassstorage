/*
 * Copyright (c) 2012 Citrix Systems, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "project.h"

#include <dlfcn.h>
#include <syslog.h>


INTERNAL uint32_t ones32(uint32_t x)
{
	x -= ((x >> 1) & 0x55555555); x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
        x = (((x >> 4) + x) & 0xf0f0f0f);
        x += (x >> 8);
        x += (x >> 16);
        return(x & 0x3f);
}

INTERNAL uint32_t floor_log2_32(uint32_t x)
{
        x |= (x >> 1);
        x |= (x >> 2);
        x |= (x >> 4);
        x |= (x >> 8);
        x |= (x >> 16);
        return(ones32(x) - 1);
}




static void
bt (void)
{
#if 0
  unsigned int level = 0;
  void *addr;
  Dl_info info;

  for (;;)
    {
      addr = __builtin_return_address (level);
      if (!addr)
        return;
      fprintf (stderr, "%d: %p", level, addr);
      if (dladdr (addr, &info))
        {
          char *base, *offset;

          base = info.dli_saddr;
          offset = addr;

          fprintf (stderr, "(%s %s+0x%x)", info.dli_fname, info.dli_sname,
                   (unsigned int) (offset - base));

        }

      fprintf (stderr, "\n");
      level++;
    }
#else
  void *ba[256];
  Dl_info info;
  int i;

  int n = backtrace (ba, sizeof (ba) / sizeof (ba[0]));
  if (!n)
    return;


  for (i = 0; i < n; ++i)
    {
      syslog (LOG_ERR, "libgmch: %d: %p", i, ba[i]);
      if (dladdr (ba[i], &info))
        {
          char *base, *offset;

          base = info.dli_saddr;
          offset = ba[i];

          syslog (LOG_ERR, "libgmch: (%s %s+0x%x)", info.dli_fname,
                  info.dli_sname, (unsigned int) (offset - base));

        }

    }
#endif
}


INTERNAL void
message (int flags, const char *file, const char *function, int line,
         const char *fmt, ...)
{
  const char *level;
  va_list ap;

  if (flags & MESSAGE_INFO)
    {
      level = "Info";
    }
  else if (flags & MESSAGE_WARNING)
    {
      level = "Warning";
    }
  else if (flags & MESSAGE_ERROR)
    {
      level = "Error";
    }
  else if (flags & MESSAGE_FATAL)
    {
      level = "Fatal";
    }

  fprintf (stderr, "%s:%s:%s:%d:", level, file, function, line);
  syslog (LOG_ERR, "%s:%s:%s:%d:", level, file, function, line);

  va_start (ap, fmt);
  vsyslog (LOG_ERR, fmt, ap);
  va_end (ap);

  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);
  fprintf (stderr,"\n");

  if (flags & MESSAGE_FATAL)
    {
      bt ();
      abort ();
    }
}

INTERNAL void *
xmalloc (size_t s)
{
  void *ret = malloc (s);
  if (!ret)
    fatal ("malloc failed");
  return ret;
}

INTERNAL void *
xrealloc (void *p, size_t s)
{
  p = realloc (p, s);
  if (!p)
    fatal ("realloc failed");
  return p;
}

INTERNAL char *
xstrdup (const char *s)
{
  char *ret = strdup (s);
  if (!ret)
    fatal ("strdup failed");
  return ret;
}


INTERNAL void
copy_and_add_null (char *dst, const char *src, size_t len)
{
  strncpy (dst, src, len-1);
  dst[len-1] = 0;
}
