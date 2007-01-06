/*
 * Multicast Routing Daemon (MRD)
 *   mrd_components.cpp
 *
 * Copyright (C) 2004, 2005, 2006
 *  Universidade de Aveiro, Instituto Telecomunicacoes - Polo Aveiro
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author:  Hugo Santos <hsantos@av.it.pt>
 */

#include <mrd/mrd.h>

#include <mrdpriv/linux/unicast_route.h>
#include <mrdpriv/linux/us_mfa.h>
#include <mrdpriv/linux/icmp_raw.h>

#if defined(__GLIBC__) && !defined(__UCLIBC__)
#include <execinfo.h>
#endif

bool mrd::prepare_os_components() {
	m_mfa = new us_mfa();

	if (!instantiate_property_b("handle-proper-bridge", false))
		return false;

#ifndef LINUX_NO_ICMPRAW
	m_icmp = new linux_icmp_raw();
#endif

	return true;
}

void mrd::prepare_second_components() {
	if (!m_rib_handler)
		m_rib_handler = new linux_unicast_router();
}

const char *mrd::loopback_interface_name() const {
	return "lo";
}

#define MAX_DEEP_BACKTRACE	32

void mrd::output_backtrace(base_stream &out) const {
#if defined(__GLIBC__) && !defined(__UCLIBC__)
	void *bt[MAX_DEEP_BACKTRACE];

	int count = backtrace(bt, MAX_DEEP_BACKTRACE);

	char **btnames = backtrace_symbols(bt, count);

	for (int i = 0; i < count; i++) {
		out.xprintf("#%i %s\n", i+1, btnames[i]);
	}

	free(btnames);
#else
	out.writeline("Backtraces aren't available in this system.");
#endif
}

char *mrd::obtain_frame_description(void *ptr) const {
#if defined(__GLIBC__) && !defined(__UCLIBC__)
	void *p[1] = { ptr };

	char **names = backtrace_symbols(p, 1);
	char *ret = strdup(names[0]);
	free(names);

	return ret;
#else
	return 0;
#endif
}

void *mrd::posix_uctx::get_current_frame() const {
#if defined(__GLIBC__)
#if defined(__i386__)
	return (void *)base->uc_mcontext.gregs[REG_EIP];
#elif defined(__mips__)
	return (void *)base->uc_mcontext.gpregs[CTX_EPC];
#endif
#endif
	return 0;
}

