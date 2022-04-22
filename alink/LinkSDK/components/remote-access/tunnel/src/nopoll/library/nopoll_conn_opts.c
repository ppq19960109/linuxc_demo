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
#include <nopoll_conn_opts.h>
#include <nopoll_private.h>

/**
 * \defgroup nopoll_conn_opts noPoll Connection Options: API to change default connection options.
 */

/**
 * \addtogroup nopoll_conn_opts
 * @{
 */

/**
 * @brief Create a new connection options object.
 *
 * @return A newly created connection options object. In general you don't have to worry about releasing this object because this is automatically done by functions using this object. However, if you call to \ref nopoll_conn_opts_set_reuse (opts, nopoll_true), then you'll have to use \ref nopoll_conn_opts_free to release the object after it is no longer used. The function may return NULL in case of memory allocation problems. Creating an object without setting anything will cause the library to provide same default behaviour as not providing it.
 */
noPollConnOpts * nopoll_conn_opts_new (void)
{
    noPollConnOpts * result;

    /* create configuration object */
    result = nopoll_new (noPollConnOpts, 1);
    if (! result)
        return NULL;

    result->reuse        = nopoll_false; /* this is not needed, just to clearly state defaults */

    result->mutex        = nopoll_mutex_create ();
    result->refs         = 1;

    return result;
}

/**
 * @brief Allows to set Cookie header content to be sent during the
 * connection handshake. If configured and the remote side server is a
 * noPoll peer, use \ref nopoll_conn_get_cookie to get this value.
 *
 * @param opts The connection option to configure.
 *
 * @param cookie_content Content for the cookie. If you pass NULL the
 * cookie is unset.
 */
void        nopoll_conn_opts_set_cookie (noPollConnOpts * opts, const char * cookie_content)
{
    if (opts == NULL)
        return;

    if (cookie_content) {
        /* configure cookie content to be sent */
        opts->cookie = nopoll_strdup (cookie_content);
    } else {
        nopoll_free (opts->cookie);
        opts->cookie = NULL;
    } /* end if */

    return;
}

/**
 * @brief Allows to set arbitrary HTTP headers and content to be sent during
 * the connection handshake.
 *
 * @note String must match the format: "\r\nheader:value\r\nheader2:value2" with
 * no trailing \r\n.
 *
 * @param opts The connection option to configure.
 *
 * @param header_string Content for the headers. If you pass NULL the
 * extra headers are unset.
 */
void        nopoll_conn_opts_set_exttunnel_headers (noPollConnOpts * opts, const char * header_string)
{
    if (opts == NULL)
        return;

    if (header_string) {
        /* configure exttunnel_header content to be sent */
        opts->exttunnel_headers = nopoll_strdup (header_string);
    } else {
        nopoll_free (opts->exttunnel_headers);
        opts->exttunnel_headers = NULL;
    } /* end if */

    return;
}


/**
 * @brief Allows to skip origin check for an incoming connection.
 *
 * This option is highly not recommended because the Origin header
 * must be provided by all WebSocket clients so the the server side
 * can check it.
 *
 * In most environments not doing so will make the connection to not succeed.
 *
 * Use this option just in development environment.
 *
 * @param opts The connection options to configure.
 *
 * @param skip_check nopoll_bool Skip header check
 *
 */
void        nopoll_conn_opts_skip_origin_check (noPollConnOpts * opts, nopoll_bool skip_check)
{
    /* configure skip origin header check */
    if (opts)
        opts->skip_origin_header_check = skip_check;

    return;
}


/**
 * @brief Allows to increase a reference to the connection options
 * provided.
 *
 * @param opts The connection option reference over which a connection
 * reference is needed.
 *
 * @return nopoll_true in the case the operation went ok, otherwise
 * nopoll_false is returned.
 */
nopoll_bool nopoll_conn_opts_ref (noPollConnOpts * opts)
{
    if (opts == NULL)
        return nopoll_false;

    /* lock the mutex */
    nopoll_mutex_lock (opts->mutex);
    if (opts->refs <= 0) {
        /* unlock the mutex */
        nopoll_mutex_unlock (opts->mutex);
        return nopoll_false;
    }

    opts->refs++;

    /* release here the mutex */
    nopoll_mutex_unlock (opts->mutex);

    return nopoll_true;
}

/**
 * @brief Allows to unref a reference acquired by \ref nopoll_conn_opts_ref
 *
 * @param opts The connection opts to release.
 */
void        nopoll_conn_opts_unref (noPollConnOpts * opts)
{
    /* call free implementation */
    nopoll_conn_opts_free (opts);
    return;
}


/**
 * @brief Set reuse-flag be used on the API receiving this
 * configuration object. By setting nopoll_true will cause the API to
 * not release the object when finished. Instead, the caller will be
 * able to use this object in additional API calls but, after
 * finishing, a call to \ref nopoll_conn_opts_set_reuse function is
 * required.
 *
 * @param opts The connection options object.
 *
 * @param reuse nopoll_true to reuse the object across calls,
 * otherwise nopoll_false to make the API function to release the
 * object when done.
 */
void nopoll_conn_opts_set_reuse        (noPollConnOpts * opts, nopoll_bool reuse)
{
    if (opts == NULL)
        return;
    opts->reuse = reuse;
    return;
}


/**
 * @brief Allows the user to configure the interface to bind the connection to.
 *
 * @param opts The connection options object.
 *
 * @param _interface The interface to bind to, or NULL for system default.
 *
 */
void nopoll_conn_opts_set_interface    (noPollConnOpts * opts, const char * _interface)
{
    if (opts == NULL)
        return;

    if (_interface) {
        /* configure interface */
        opts->_interface = nopoll_strdup (_interface);
    } else {
        nopoll_free (opts->_interface);
        opts->_interface = NULL;
    } /* end if */

    return;
}

void __nopoll_conn_opts_free_common  (noPollConnOpts * opts)
{
    if (opts == NULL)
        return;

    /* acquire here the mutex */
    nopoll_mutex_lock (opts->mutex);

    opts->refs--;
    if (opts->refs != 0) {
        /* release here the mutex */
        nopoll_mutex_unlock (opts->mutex);
        return;
    }
    /* release here the mutex */
    nopoll_mutex_unlock (opts->mutex);
    /* cookie */
    nopoll_free (opts->cookie);

    /* interface */
    nopoll_free (opts->_interface);

    if (opts->exttunnel_headers)
        nopoll_free (opts->exttunnel_headers);

    /* release mutex */
    nopoll_mutex_destroy (opts->mutex);
    nopoll_free (opts);
    return;
}

/**
 * @brief Allows to release a connection object reported by \ref nopoll_conn_opts_new
 *
 * IMPORTANT NOTE: do not use this function over a \ref noPollConnOpts if it is not flagged with \ref nopoll_conn_opts_set_reuse (opts, nopoll_true).
 *
 * Default behaviour provided by the API implies that every connection
 * options object created by \ref nopoll_conn_opts_new is
 * automatically released by the API consuming that object.
 */
void nopoll_conn_opts_free (noPollConnOpts * opts)
{
    __nopoll_conn_opts_free_common (opts);
    return;
} /* end if */

/**
 * @internal API. Do not use it. It may change at any time without any
 * previous indication.
 */
void __nopoll_conn_opts_release_if_needed (noPollConnOpts * options)
{
    if (! options)
        return;
    if (options && options->reuse)
        return;
    __nopoll_conn_opts_free_common (options);
    return;
}

/* @} */
