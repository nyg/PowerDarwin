/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/* The forms that CF-476.19's CFInternal.h gives the Objective-C bridging macros in a CoreFoundation without Objective-C, under the names CFOpenDirectory uses, and the declaration of _CFRuntimeBridgeClasses, which CF-476.19's CFRuntime.c defines and exports as a function that does nothing. Written on 2026-11-28. */

#ifndef _CFBRIDGINGPRIV_H_
#define _CFBRIDGINGPRIV_H_

#include <CoreFoundation/CFBase.h>

#define CF_IS_OBJC(typeID, obj) (false)

#define CF_OBJC_VOIDCALL(obj, sel, ...)
#define CF_OBJC_CALL(rettype, retvar, obj, sel, ...)
#define CF_OBJC_FUNCDISPATCH(typeID, rettype, obj, sel, ...)

CF_EXPORT void _CFRuntimeBridgeClasses(CFTypeID cf_typeID, const char *objc_classname);

#endif
