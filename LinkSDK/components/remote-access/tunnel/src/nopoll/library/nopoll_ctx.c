/*
 *  LibNoPoll: A websocket library
 *  Copyright (C) 2017 Advanced Software Production Line, S.L.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation; either version 2.1
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307 USA
 *
 *  You may find a copy of the license under this software is released
 *  at COPYING file. This is LGPL software: you are welcome to develop
 *  proprietary applications using this library without any royalty or
 *  fee but returning back any change, improvement or addition in the
 *  form of source code, project image, documentation patches, etc.
 *
 *  For commercial support on build Websocket enabled solutions
 *  contact us:
 *
 *      Postal address:
 *         Advanced Software Production Line, S.L.
 *         Av. Juan Carlos I, Nº13, 2ºC
 *         Alcalá de Henares 28806 Madrid
 *         Spain
 *
 *      Email address:
 *         info@aspl.es - http://www.aspl.es/nopoll
 */
#include <nopoll_ctx.h>
#include <nopoll_private.h>
#include <signal.h>

/**
 * \defgroup nopoll_ctx noPoll Context: context handling functions used by the library
 */

/**
 * \addtogroup nopoll_ctx
 * @{
 */
void __nopoll_ctx_sigpipe_do_nothing (int _signal)
{
#if !defined(NOPOLL_OS_WIN32)
    /* do nothing sigpipe handler to be able to manage EPIPE error
     * returned by write ops. */

    /* the following line is to ensure ancient glibc version that
     * restores to the default handler once the signal handling is
     * executed. */
    signal (SIGPIPE, __nopoll_ctx_sigpipe_do_nothing);
#endif
    return;
}


/**
 * @brief Creates an empty Nopoll context.
 */
noPollCtx * nopoll_ctx_new (void) {
    noPollCtx * result;

    /* call to create context after checkign WinSock */
    result = nopoll_new (noPollCtx, 1);
    if (result == NULL)
        return NULL;

#if defined(NOPOLL_OS_WIN32)
    if (! nopoll_win32_init (result)) {
        return NULL;
    } /* end if */
#endif

    /* set initial reference */
    result->refs = 1;

    /* 20 seconds for connection timeout */
    result->conn_connect_std_timeout = 20000000;

    /* default log initialization */
    result->not_executed  = nopoll_true;
    result->debug_enabled = nopoll_false;

    /* colored log */
    result->not_executed_color  = nopoll_true;
    result->debug_color_enabled = nopoll_false;

    /* setup default protocol version */
    result->protocol_version = 13;

    /* create mutexes */
    result->ref_mutex = nopoll_mutex_create ();
    result->read_handler = NULL;
    result->write_handler = NULL;
    result->user_data = NULL;

#if !defined(NOPOLL_OS_WIN32)
    /* install sigpipe handler */
    signal (SIGPIPE, __nopoll_ctx_sigpipe_do_nothing);
#endif

    return result;
}

/**
 * @brief Allows to acquire a reference to the provided context. This
 * reference is released by calling to \ref nopoll_ctx_unref.
 *
 * @param ctx The context to acquire a reference.
 *
 * @return The function returns nopoll_true in the case the reference
 * was acquired, otherwise nopoll_false is returned.
 */
nopoll_bool    nopoll_ctx_ref (noPollCtx * ctx)
{
    /* return false value */
    nopoll_return_val_if_fail (ctx, ctx, nopoll_false);

    /* acquire mutex here */
    nopoll_mutex_lock (ctx->ref_mutex);

    ctx->refs++;

    /* release mutex here */
    nopoll_mutex_unlock (ctx->ref_mutex);

    return nopoll_true;
}


/**
 * @brief allows to release a reference acquired to the provided
 * noPoll context.
 *
 * @param ctx The noPoll context reference to release..
 */
void           nopoll_ctx_unref (noPollCtx * ctx)
{
    nopoll_return_if_fail (ctx, ctx);

    /* acquire mutex here */
    nopoll_mutex_lock (ctx->ref_mutex);

    ctx->refs--;
    if (ctx->refs != 0) {
        /* release mutex here */
        nopoll_mutex_unlock (ctx->ref_mutex);
        return;
    }
    /* release mutex here */
    nopoll_mutex_unlock (ctx->ref_mutex);

    nopoll_log (ctx, NOPOLL_LEVEL_DEBUG, "Releasing no poll context %p (%d)", ctx, ctx->refs);


    /* release mutex */
    nopoll_mutex_destroy (ctx->ref_mutex);

    nopoll_free (ctx);
    return;
}

/**
 * @brief Allows to get current reference counting for the provided
 * context.
 *
 * @param ctx The context the reference counting is being requested.
 *
 * @return The reference counting or -1 if it fails.
 */
int            nopoll_ctx_ref_count (noPollCtx * ctx)
{
    int result;
    if (! ctx)
        return -1;

    /* lock */
    nopoll_mutex_lock (ctx->ref_mutex);

    result = ctx->refs;

    /* unlock */
    nopoll_mutex_unlock (ctx->ref_mutex);

    return result;
}

/**
 * @brief Allows to change the protocol version that is send in all
 * client connections created under the provided context and the
 * protocol version accepted by listener created under this context
 * too.
 *
 * This is a really basic (mostly fake) protocol version support
 * because it only allows to change the version string sent (but
 * nothing more for now). It is useful for testing purposes.
 *
 * @param ctx The noPoll context where the protocol version change
 * will be applied.
 *
 * @param version The value representing the protocol version. By
 * default this function isn't required to be called because it
 * already has the right protocol value configured (13).
 */
void           nopoll_ctx_set_protocol_version (noPollCtx * ctx, int version)
{
    /* check input data */
    nopoll_return_if_fail (ctx, ctx || version);

    /* setup the new protocol version */
    ctx->protocol_version = version;

    return;
}

/* @} */
void           nopoll_ctx_set_read_write_handle(noPollCtx *ctx, noPollRead read_handle, noPollRead write_handle)
{
    ctx->read_handler = read_handle;
    ctx->write_handler = write_handle;
}

void           nopoll_ctx_set_userdata(noPollCtx *ctx, void *userdata)
{
    ctx->user_data = userdata;
}
