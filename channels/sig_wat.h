#ifndef _SIG_WAT_H
#define _SIG_WAT_H
/*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (C) 1999 - 2009, Digium, Inc.
 *
 * Mark Spencer <markster@digium.com>
 *
 * See http://www.asterisk.org for more information about
 * the Asterisk project. Please do not directly contact
 * any of the maintainers of this project for assistance;
 * the project provides a web site, mailing lists and IRC
 * channels for your use.
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2. See the LICENSE file
 * at the top of the source tree.
 */

/*! \file
 *
 * \brief Interface header for Wireless AT commands signaling module
 *
 * \author David Yat Sin <dyatsin@sangoma.com>
 */


#include "asterisk/channel.h"
#include "asterisk/frame.h"
#include "asterisk/event.h"
#include <libwat.h>
#include <dahdi/user.h>

//DAVIDY remove this define later
#define WAT_NOT_IMPL ast_log(LOG_WARNING, "Function not implemented (%s:%s:%d)\n", __FILE__, __FUNCTION__, __LINE__);


enum sig_wat_law {
	SIG_WAT_DEFLAW = 0,
	SIG_WAT_ULAW,
	SIG_WAT_ALAW
};

enum sig_wat_tone {
	SIG_WAT_TONE_RINGTONE = 0,
	SIG_WAT_TONE_STUTTER,
	SIG_WAT_TONE_CONGESTION,
	SIG_WAT_TONE_DIALTONE,
	SIG_WAT_TONE_DIALRECALL,
	SIG_WAT_TONE_INFO,
	SIG_WAT_TONE_BUSY,
};

struct sig_wat_span;

struct sig_wat_callback {
	/* Unlock the private in the signalling private structure.  This is used for three way calling madness. */
	void (* const unlock_private)(void *pvt);
	/* Lock the private in the signalling private structure.  ... */
	void (* const lock_private)(void *pvt);
	/* Do deadlock avoidance for the private signaling structure lock.  */
	void (* const deadlock_avoidance_private)(void *pvt);
	/* Function which is called back to handle any other DTMF events that are received.  Called by analog_handle_event.  Why is this
	* important to use, instead of just directly using events received before they are passed into the library?  Because sometimes,
	* (CWCID) the library absorbs DTMF events received. */
	//void (* const handle_dtmf)(void *pvt, struct ast_channel *ast, enum analog_sub analog_index, struct ast_frame **dest);

	//int (* const dial_digits)(void *pvt, enum analog_sub sub, struct analog_dialoperation *dop);
	int (* const play_tone)(void *pvt, enum sig_wat_tone tone); /* DAVIDY: Do I need this? */

	int (* const set_echocanceller)(void *pvt, int enable);
	int (* const train_echocanceller)(void *pvt);				/* DAVIDY: Do I need this? */
	int (* const dsp_reset_and_flush_digits)(void *pvt);		/* DAVIDY: Do I need this? */

	struct ast_channel * (* const new_ast_channel)(void *pvt, int state, int sub, const struct ast_channel *requestor);

	void (* const fixup_chans)(void *old_chan, void *new_chan);	/* DAVIDY: Do I need this? */

	void (* const handle_sig_exception)(struct sig_wat_span *wat);
	void (* const set_alarm)(void *pvt, int in_alarm);
	void (* const set_dialing)(void *pvt, int is_dialing);
	void (* const set_digital)(void *pvt, int is_digital);
	void (* const set_callerid)(void *pvt, const struct ast_party_caller *caller);
	void (* const set_dnid)(void *pvt, const char *dnid);
	void (* const set_rdnis)(void *pvt, const char *rdnis);	/* DAVIDY: Do I need this? */
	void (* const queue_control)(void *pvt, int subclass);	/* DAVIDY: Do I need this? */
	int (* const new_nobch_intf)(struct sig_wat_span *wat);	/* DAVIDY: Do I need this? */
	void (* const init_config)(void *pvt, struct sig_wat_span *wat);
	const char *(* const get_orig_dialstring)(void *pvt);
	void (* const make_cc_dialstring)(void *pvt, char *buf, size_t buf_size);
	void (* const update_span_devstate)(struct sig_wat_span *wat);

	void (* const open_media)(void *pvt);

	/*!
	 * \brief Post an AMI B channel association event.
	 *
	 * \param pvt Private structure of the user of this module.
	 * \param chan Channel associated with the private pointer
	 *
	 * \return Nothing
	 */
	void (* const ami_channel_event)(void *pvt, struct ast_channel *chan); /* DAVIDY: Do I need this? */

	/*! Reference the parent module. */
	void (*module_ref)(void);
	/*! Unreference the parent module. */
	void (*module_unref)(void);
};

struct sig_wat_chan;

struct sig_wat_call {
	uint8_t wat_call_id; /*!< Id used by libwat for this call */

	int cid_ton;
	char cid_num[AST_MAX_EXTENSION];

	
	struct sig_wat_chan *chan;
};

struct sig_wat_chan {
	struct sig_wat_span *wat;
	struct sig_wat_callback *calls;
	void *chan_pvt;					/*!< Private structure of the user of this module. */	

	struct sig_wat_call *call;	/*!< Active call if there is one */

	char context[AST_MAX_CONTEXT];
	char mohinterpret[MAX_MUSICCLASS];
	int channel;					/*!< Channel Number or CRV */

	struct ast_channel *owner;
};

struct sig_wat_span {
	int fd;						/*!< FD for the uart channel */
	struct sig_wat_chan *pvt;	/*!< Member channel pvt struct */

	pthread_t master;			/*!< Thread of master */	
	
	int sigchanavail;			/*!< Whether channel is available */

	int span;					/*!< span number put into user output messages */
	int wat_span_id;			/*!< Identifier used by libwat for this span */

	wat_span_config_t wat_cfg;

	struct sig_wat_callback *calls;	

	ast_mutex_t lock;			/*!< libwat access mutex */ /* DAVIDY do I need this? */
};

int sig_wat_start_wat(struct sig_wat_span *wat);
void sig_wat_stop_wat(struct sig_wat_span *wat);
void sig_wat_init_wat(struct sig_wat_span *wat);


int sig_wat_call(struct sig_wat_chan *p, struct ast_channel *ast, char *rdest);
int sig_wat_answer(struct sig_wat_chan *p, struct ast_channel *ast);
int sig_wat_hangup(struct sig_wat_chan *p, struct ast_channel *ast);

void wat_event_alarm(struct sig_wat_span *wat, int before_start_wat);
void wat_event_noalarm(struct sig_wat_span *wat, int before_start_wat);

void sig_wat_load(int maxspans);
void sig_wat_unload(void);

struct sig_wat_chan *sig_wat_chan_new(void *pvt_data, struct sig_wat_callback *callback, struct sig_wat_span *wat, int channo);


#endif /* _SIG_WAT_H */
