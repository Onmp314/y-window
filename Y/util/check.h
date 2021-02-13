#ifndef CHECK_H
#define CHECK_H

extern const char *checkName;
extern const char *checkModule;

#ifndef CHECK_STOP
#define CHECK_STOP return 1
#endif

#define CHECK_THAT(exp)     do                                                               \
                              {                                                              \
                                if (!(exp))                                                  \
                                  {                                                          \
                                    fprintf (stderr, "check %s %s: (%s:%d)\nFAILED AT %s\n", \
                                             checkName, checkModule, __FILE__, __LINE__,     \
                                             #exp );                                         \
                                    CHECK_STOP;                                              \
                                  }                                                          \
                              }                                                              \
                            while (0)
                              

#endif

/* arch-tag: a1905a9f-8eda-468c-8acc-9986160f96b0
 */
