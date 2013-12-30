#ifndef HAL_MAILBOX_H
#define HAL_MAILBOX_H

#define MBOX_SBL_SHELL     ((unsigned long)0x53544159)  // 'STAY' in SBL and run command shell
#define MBOX_SBL_GO_APP    ((unsigned long)0x4732474F)  // 'G2GO' good to go to App

// this is the mailbox value that tells the boot code to flash the downloaded image
// even though the operational image may be sane.
#define MBOX_OAD_ENABLE    ((unsigned long)0x454E424C)  // 'ENBL' enable downloaded code

/*
 * This is overlayed with the list of MAC timer callbacks for CC MAC. This
 * won't matter because it is read only after reset. In the boot code the
 * MAC stuff doesn't matter. In the application it is read in ZMain:main()
 * before the MAC init runs. Currently no one cares about what is read. We
 * can add an API if we need it. When the reset command executes it's a
 * don't-care when we write over the possible MAC stuff since we;'re
 * resetting anyway.
 */
typedef struct mbox_s {
  volatile unsigned long BootRead;
  volatile unsigned long AppRead;
} mboxMsg_t;

__no_init mboxMsg_t mboxMsg @ 0xE000;

#endif
