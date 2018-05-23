/*
 * Strings that appear in header lines should be kept short.
 *
 * Current source version: 1.30c
 * If WinCenter support is configured, append "+".
 * If you're left-handed and the temperature is above freezing, append xyzzy.
 * But I digress.
 */

#define VERSION "2.0.0"            /* Version string    */
#define RELEASE 'a'                /* Release character */
#define RELEASE_DATE "07-AUG-10"   /* Release date      */

#ifndef lint

#ifdef __STDC__
static
#endif /* __STDC__ */
char fs_str[] = "@(#)LinkStat Server v2.00a+\n";
#define HDR_VERSION "2.00a+"

#ifdef __STDC__
static
#endif /* __STDC__ */
char rcsid[] = "@(#)$Header: /home/kevcla/src/linkstat/RCS/version.c,v 1.19 2010/08/05 04:29:27 kevcla Exp $";

#ifdef __STDC__
static
#endif /* __STDC__ */
char id_str[] = "@(#)Copyright 1998 Kevin Clark\n";

#endif /* lint */

extern char datecompiled[];

/****************************************************************************
* Function Name      :   version_display_info
* Module ID          :   V(1)
* 
* Purpose            :   To display program version information, and also
*                        to give enough information on the compilation
*                        to pinpoint the source versions.
* 
* Method             :   Uses the RCS ID string contained within the module.
* 
* Usage              :   main (M1)
* 
* External References:   (none)
* 
* Arguments          :   verbose: (data_in)
*                                The amount of information to display.
* 
* Return Value       :   (none)
* 
* Input Assertions   :   rcsid must be defined.
* 
* Output Assertions  :   (none)
* 
* Variables          :   (none)
* 
* Authors(s)         :   Kevin Clark
* 
* Remarks            :   This assumes the developer uses RCS.
\***************************************************************************/
void
version_display_info(verbose)
int verbose;
{
    printf("\nVersion %s (%c)\t\t\t\t\t%s\n\n", VERSION, RELEASE,
	   RELEASE_DATE);

    if (verbose > 1) {
	char time_str[20], date_str[20], version_str[10], skip_it[40], person;

	sscanf(rcsid, "%s%s%s%s%s %c", skip_it, skip_it, version_str,
	       date_str, time_str, &person);
	printf("Source Distribution File: rcsid-%s%c\n", version_str,
	       person);
	printf("Source Distribution Date: %s\n", date_str);
	printf("Source Distribution Time: %s\n", time_str);
#ifdef UNAME
	printf("Source Platform Basename: %s\n\n", UNAME);
#endif
	printf("Compiled on %s\n\n", datecompiled);
    }
}

/****************************************************************************
* Function Name      :   version_get_str
* Module ID          :   V(1)
* 
* Purpose            :   To retrieve program version information.
* 
* Method             :   Uses the VERSION macro contained within this module.
* 
* Usage              :   main (M1)
* 
* External References:   (none)
* 
* Arguments          :   (none)
* 
* Return Value       :   char *
*                                The string containing version information.
* 
* Input Assertions   :   (none)
* 
* Output Assertions  :   (none)
* 
* Variables          :   (none)
* 
* Authors(s)         :   Kevin Clark
* 
* Remarks            :   (none)
\***************************************************************************/
char *
version_get_str()
{
    return (VERSION);
}

/****************************************************************************
* Function Name      :   version_get_rel
* Module ID          :   V(1)
* 
* Purpose            :   To retrieve program release character.
* 
* Method             :   Uses the RELEASE macro contained within this module.
* 
* Usage              :   main (M1)
* 
* External References:   (none)
* 
* Arguments          :   (none)
* 
* Return Value       :   char
*                                The release character.
* 
* Input Assertions   :   (none)
* 
* Output Assertions  :   (none)
* 
* Variables          :   (none)
* 
* Authors(s)         :   Kevin Clark
* 
* Remarks            :   (none)
\***************************************************************************/
char
version_get_rel()
{
    return (RELEASE);
}

/****************************************************************************
* Function Name      :   version_get_rel_date
* Module ID          :   V(1)
* 
* Purpose            :   To retrieve program release date.
* 
* Method             :   Uses the RELEASE_DATE macro contained within this module.
* 
* Usage              :   main (M1)
* 
* External References:   (none)
* 
* Arguments          :   (none)
* 
* Return Value       :   char *
*                                The release date.
* 
* Input Assertions   :   (none)
* 
* Output Assertions  :   (none)
* 
* Variables          :   (none)
* 
* Authors(s)         :   Kevin Clark
* 
* Remarks            :   (none)
\***************************************************************************/
char *
version_get_rel_date()
{
    return (RELEASE_DATE);
}

