#
#
# Makefile.am:
#
#
# $Id:$
#
# $Log:$
#
#
#

#
# Copyright (c) 2012 Citrix Systems, Inc.
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#

SUBDIRS=src test app

EXTRA_DIST= version-major version-minor version-micro version-files version-md5sums

bin_SCRIPTS = libusbmassstorage-config

pkgconfigdir=${LIBDIR}/pkgconfig
pkgconfig_DATA= libusbmassstorage.pc

libusbmassstorage.pc.src:libusbmassstorage.pc.src.in
libusbmassstorage.pc:libusbmassstorage.pc.src
	@SED@ -f version.sed < libusbmassstorage.pc.src > libusbmassstorage.pc || rm -f libusbmassstorage.pc

libusbmassstorage-config.src:libusbmassstorage-config.src.in
libusbmassstorage-config:libusbmassstorage-config.src version.sed
	@SED@ -f version.sed < libusbmassstorage-config.src > libusbmassstorage-config || rm -f libusbmassstorage-config
	chmod +x $@
	

VFD=${srcdir}
VF=${shell cat ${VFD}/version-files}
VFS=${VF:%=${VFD}/%}
VCHK=${shell cat ${VFS} | @MD5SUM@ | @AWK@ '{print $$1 }' }
VNUM=${shell @GREP@ ${VCHK} ${VFD}/version-md5sums | @AWK@ '{ print $$2 }'  }
VDEF=${shell echo `cat ${VFD}/version-major`.`cat ${VFD}/version-minor`.`cat ${VFD}/version-micro` }

version.sed: $(VFD)/version-files $(VFD)/version-major \
	$(VFD)/version-minor $(VFD)/version-micro \
	$(VFD)/version-md5sums ${VFS} Makefile
	if [ .${VNUM} = . ]; then \
		echo "s/%VERSION%/${VDEF}-E/g" > version.sed; \
	else \
		echo "s/%VERSION%/${VNUM}/g" > version.sed; \
	fi

