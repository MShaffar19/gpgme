/* decrypt-verify.c -  decrypt and verify functions
 *	Copyright (C) 2000 Werner Koch (dd9jn)
 *      Copyright (C) 2001 g10 Code GmbH
 *
 * This file is part of GPGME.
 *
 * GPGME is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPGME is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "util.h"
#include "context.h"
#include "ops.h"

static void
decrypt_verify_status_handler (GpgmeCtx ctx, GpgStatusCode code, char *args)
{
  _gpgme_decrypt_status_handler (ctx, code, args);
  _gpgme_verify_status_handler (ctx, code, args);
}

GpgmeError
gpgme_op_decrypt_verify_start (GpgmeCtx ctx, GpgmeData ciph, GpgmeData plain)
{
  return _gpgme_decrypt_start (ctx, ciph, plain,
			       decrypt_verify_status_handler);
}

/**
 * gpgme_op_decrypt_verify:
 * @ctx: The context
 * @in: ciphertext input
 * @out: plaintext output
 * 
 * This function decrypts @in to @out and performs a signature check.
 * Other parameters are take from the context @c.
 * The function does wait for the result.
 * 
 * Return value:  0 on success or an errorcode. 
 **/
GpgmeError
gpgme_op_decrypt_verify (GpgmeCtx ctx,
			 GpgmeData in, GpgmeData out,
			 GpgmeSigStat *r_stat)
{
  GpgmeError err = gpgme_op_decrypt_verify_start (ctx, in, out);
  if (!err)
    {
      gpgme_wait (ctx, 1);
      if (!ctx->result.decrypt || !ctx->result.verify)
	err = mk_error (General_Error);
      else if (ctx->out_of_core)
	err = mk_error (Out_Of_Core);
      else
	{
	  err = _gpgme_decrypt_result (ctx);
          if (! err)
	    *r_stat = _gpgme_intersect_stati (ctx->result.verify);
	}
      ctx->pending = 0;
    }
  return err;
}