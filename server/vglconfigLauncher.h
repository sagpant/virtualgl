/* Copyright (C)2007 Sun Microsystems, Inc.
 * Copyright (C)2014 D. R. Commander
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

#ifndef __VGLCONFIGLAUNCHER_H__
#define __VGLCONFIGLAUNCHER_H__

#include <X11/Xlib.h>
#include <unistd.h>
#include "Mutex.h"
#include "Thread.h"
#include "Log.h"
#include "fakerconfig.h"


namespace vglserver
{
	class vglconfigLauncher : public vglutil::Runnable
	{
		public:

			static vglconfigLauncher *instance(void)
			{
				if(instancePtr==NULL)
				{
					vglutil::CS::SafeLock l(instanceMutex);
					if(instancePtr==NULL) instancePtr=new vglconfigLauncher;
				}
				return instancePtr;
			}

			void popup(Display *dpy_, int shmid_)
			{
				if(!dpy_ || shmid_==-1) _throw("Invalid argument");
				vglutil::CS::SafeLock l(popupMutex);
				if(thread) return;
				dpy=dpy_;  shmid=shmid_;
				errifnot(thread=new vglutil::Thread(this));
				thread->start();
			}

			void run(void)
			{
				try
				{
					char commandLine[1024];
					unsetenv("LD_PRELOAD");
					unsetenv("LD_PRELOAD_32");
					unsetenv("LD_PRELOAD_64");
					sprintf(commandLine, "%s -display %s -shmid %d -ppid %d",
						fconfig.config, DisplayString(dpy), shmid, getpid());
					if(system(commandLine)==-1) _throwunix();
				}
				catch(vglutil::Error &e)
				{
					vglout.println("Error invoking vglconfig--\n%s", e.getMessage());
				}
				vglutil::CS::SafeLock l(popupMutex);
				thread->detach();  delete thread;  thread=NULL;
			}

		private:

			vglconfigLauncher(void): thread(NULL), dpy(NULL), shmid(-1)
			{
			}

			virtual ~vglconfigLauncher(void) {}

			int unsetenv(const char *name)
			{
				char *str=NULL;
				if(!name) { errno=EINVAL;  return -1; }
				if(strlen(name)<1 || strchr(name, '='))
				{
					errno=EINVAL;  return -1;
				}
				if(!getenv(name)) return -1;
				if((str=(char *)malloc(strlen(name)+2))==NULL)
				{
					errno=ENOMEM;  return -1;
				}
				sprintf(str, "%s=", name);
				putenv(str);
				strcpy(str, "=");
				putenv(str);
				return 0;
			}

			static vglconfigLauncher *instancePtr;
			static vglutil::CS instanceMutex;
			static vglutil::CS popupMutex;
			vglutil::Thread *thread;
			Display *dpy;
			int shmid;
	};
}


#define vglpopup(dpy, shmid) \
	((*(vglconfigLauncher::instance())).popup(dpy, shmid))

#endif // __VGLCONFIGLAUNCHER_H__
