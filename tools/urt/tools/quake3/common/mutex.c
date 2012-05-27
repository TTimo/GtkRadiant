/*
This code is based on source provided under the terms of the Id Software 
LIMITED USE SOFTWARE LICENSE AGREEMENT, a copy of which is included with the
GtkRadiant sources (see LICENSE_ID). If you did not receive a copy of 
LICENSE_ID, please contact Id Software immediately at info@idsoftware.com.

All changes and additions to the original source which have been developed by
other contributors (see CONTRIBUTORS) are provided under the terms of the
license the contributors choose (see LICENSE), to the extent permitted by the
LICENSE_ID. If you did not receive a copy of the contributor license,
please contact the GtkRadiant maintainers at info@gtkradiant.com immediately.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "cmdlib.h"
#include "qthreads.h"
#include "mutex.h"

/*
===================================================================

WIN32

===================================================================
*/
#ifdef WIN32

#define	USED

#include <windows.h>

void MutexLock (mutex_t *m)
{
	CRITICAL_SECTION *crit;

	if (!m)
		return;
	crit = (CRITICAL_SECTION *) m;
	EnterCriticalSection (crit);
}

void MutexUnlock (mutex_t *m)
{
	CRITICAL_SECTION *crit;

	if (!m)
		return;
	crit = (CRITICAL_SECTION *) m;
	LeaveCriticalSection (crit);
}

mutex_t *MutexAlloc(void)
{
	CRITICAL_SECTION *crit;

	if (numthreads == 1)
		return NULL;
	crit = (CRITICAL_SECTION *) safe_malloc(sizeof(CRITICAL_SECTION));
	InitializeCriticalSection (crit);
	return (void *) crit;
}

#endif

/*
===================================================================

OSF1

===================================================================
*/

#ifdef __osf__
#define	USED

#include <pthread.h>

void MutexLock (mutex_t *m)
{
	pthread_mutex_t	*my_mutex;

	if (!m)
		return;
	my_mutex = (pthread_mutex_t *) m;
	pthread_mutex_lock (my_mutex);
}

void MutexUnlock (mutex_t *m)
{
	pthread_mutex_t	*my_mutex;

	if (!m)
		return;
	my_mutex = (pthread_mutex_t *) m;
	pthread_mutex_unlock (my_mutex);
}

mutex_t *MutexAlloc(void)
{
	pthread_mutex_t	*my_mutex;
	pthread_mutexattr_t	mattrib;

	if (numthreads == 1)
		return NULL;
	my_mutex = safe_malloc (sizeof(*my_mutex));
	if (pthread_mutexattr_create (&mattrib) == -1)
		Error ("pthread_mutex_attr_create failed");
	if (pthread_mutexattr_setkind_np (&mattrib, MUTEX_FAST_NP) == -1)
		Error ("pthread_mutexattr_setkind_np failed");
	if (pthread_mutex_init (my_mutex, mattrib) == -1)
		Error ("pthread_mutex_init failed");
	return (void *) my_mutex;
}

#endif

/*
===================================================================

IRIX

===================================================================
*/

#ifdef _MIPS_ISA 
#define	USED

#include <task.h>
#include <abi_mutex.h>
#include <sys/types.h>
#include <sys/prctl.h>

void MutexLock (mutex_t *m)
{
	abilock_t *lck;

	if (!m)
		return;
	lck = (abilock_t *) m;
	spin_lock (lck);
}

void MutexUnlock (mutex_t *m)
{
	abilock_t *lck;

	if (!m)
		return;
	lck = (abilock_t *) m;
	release_lock (lck);
}

mutex_t *MutexAlloc(void)
{
	abilock_t *lck;

	if (numthreads == 1)
		return NULL;
	lck = (abilock_t *) safe_malloc(sizeof(abilock_t));
	init_lock (lck);
	return (void *) lck;
}

#endif

/*
=======================================================================

  SINGLE THREAD

=======================================================================
*/

#ifndef USED

void MutexLock (mutex_t *m)
{
}

void MutexUnlock (mutex_t *m)
{
}

mutex_t *MutexAlloc(void)
{
	return NULL;
}

#endif
