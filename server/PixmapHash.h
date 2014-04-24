/* Copyright (C)2004 Landmark Graphics Corporation
 * Copyright (C)2011, 2014 D. R. Commander
 *
 * This library is free software and may be redistributed and/or modified under
 * the terms of the wxWindows Library License, Version 3.1 or (at your option)
 * any later version.  The full license is in the LICENSE.txt file included
 * with this distribution.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * wxWindows Library License for more details.
 */

#ifndef __PIXMAPHASH_H__
#define __PIXMAPHASH_H__

#include "VirtualPixmap.h"
#include "Hash.h"


#define _Hash Hash<char *, Pixmap, VirtualPixmap *>

// This maps a 2D pixmap ID on the 2D X Server to a VirtualPixmap instance,
// which encapsulates the corresponding 3D pixmap on the 3D X Server

namespace vglserver
{
	class PixmapHash : public _Hash
	{
		public:

			static PixmapHash *instance(void)
			{
				if(instancePtr==NULL)
				{
					vglutil::CS::SafeLock l(instanceMutex);
					if(instancePtr==NULL) instancePtr=new PixmapHash;
				}
				return instancePtr;
			}

			static bool isAlloc(void) { return (instancePtr!=NULL); }

			void add(Display *dpy, Pixmap pm, VirtualPixmap *vpm)
			{
				if(!dpy || !pm) _throw("Invalid argument");
				char *dpystring=strdup(DisplayString(dpy));
				if(!_Hash::add(dpystring, pm, vpm))
					free(dpystring);
			}

			VirtualPixmap *find(Display *dpy, Pixmap pm)
			{
				if(!dpy || !pm) return NULL;
				return _Hash::find(DisplayString(dpy), pm);
			}

			Pixmap reverseFind(GLXDrawable glxd)
			{
				if(!glxd) return 0;
				HashEntry *ptr=NULL;
				vglutil::CS::SafeLock l(mutex);
				if((ptr=_Hash::findEntry(NULL, glxd))!=NULL)
					return ptr->key2;
				return 0;
			}

			void remove(Display *dpy, GLXPixmap glxpm)
			{
				if(!dpy || !glxpm) _throw("Invalid argument");
				_Hash::remove(DisplayString(dpy), glxpm);
			}

		private:

			~PixmapHash(void)
			{
				_Hash::kill();
			}

			void detach(HashEntry *entry)
			{
				if(entry && entry->key1) free(entry->key1);
				if(entry && entry->value) delete((VirtualPixmap *)entry->value);
			}

			bool compare(char *key1, Pixmap key2, HashEntry *entry)
			{
				VirtualPixmap *vpm=entry->value;
				return (
					(key1 && !strcasecmp(key1, entry->key1)
							&& (key2==entry->key2 || (vpm && key2==vpm->getGLXDrawable())))
						|| (key1==NULL && key2==vpm->getGLXDrawable())
				);
			}

			static PixmapHash *instancePtr;
			static vglutil::CS instanceMutex;
	};
}

#undef _Hash


#define pmhash (*(PixmapHash::instance()))

#endif // __PIXMAPHASH_H__
