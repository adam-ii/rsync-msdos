/*
**  wildmatch test suite.
*/

/*#define COMPARE_WITH_FNMATCH*/

#define WILD_TEST_ITERATIONS
#include "lib/wildmatch.c"

#include "popt.h"

#ifdef COMPARE_WITH_FNMATCH
#include <fnmatch.h>

int fnmatch_errors = 0;
#endif

int wildmatch_errors = 0;

typedef char bool;

#define false 0
#define true 1

int output_iterations = 0;

static struct poptOption long_options[] = {
  /* longName, shortName, argInfo, argPtr, value, descrip, argDesc */
  {"iterations",     'i', POPT_ARG_NONE,   &output_iterations, 0, 0, 0},
  {0,0,0,0, 0, 0, 0}
};

/* match just at the start of string (anchored tests) */
static void
run_test(int line, bool matches, bool same_as_fnmatch,
	 const char *text, const char *pattern)
{
    bool matched;
#ifdef COMPARE_WITH_FNMATCH
    bool fn_matched;
    int flags = strstr(pattern, "**")? 0 : FNM_PATHNAME;
#else
    same_as_fnmatch = 0; /* Get rid of unused-variable compiler warning. */
#endif

    matched = wildmatch(pattern, text);
#ifdef COMPARE_WITH_FNMATCH
    fn_matched = !fnmatch(pattern, text, flags);
#endif
    if (matched != matches) {
	printf("wildmatch failure on line %d:\n  %s\n  %s\n  expected %s match\n",
	       line, text, pattern, matches? "a" : "NO");
	wildmatch_errors++;
    }
#ifdef COMPARE_WITH_FNMATCH
    if (fn_matched != (matches ^ !same_as_fnmatch)) {
	printf("fnmatch disagreement on line %d:\n  %s\n  %s\n  expected %s match\n",
	       line, text, pattern, matches ^ !same_as_fnmatch? "a" : "NO");
	fnmatch_errors++;
    }
#endif
    if (output_iterations) {
	printf("%d: \"%s\" iterations = %d\n", line, pattern,
	       wildmatch_iteration_count);
    }
}

int
main(int argc, char **argv)
{
    char buf[2048], *s, *string[2], *end[2];
    FILE *fp;
    int opt, line, i, flag[2];
    poptContext pc = poptGetContext("wildtest", argc, (const char**)argv,
				    long_options, 0);

    while ((opt = poptGetNextOpt(pc)) != -1) {
	switch (opt) {
	  default:
	    fprintf(stderr, "%s: %s\n",
		    poptBadOption(pc, POPT_BADOPTION_NOALIAS),
		    poptStrerror(opt));
	    exit(1);
	}
    }

    if ((fp = fopen("wildtest.txt", "r")) == NULL) {
	fprintf(stderr, "Unable to open wildtest.txt.\n");
	exit(1);
    }

    line = 0;
    while (fgets(buf, sizeof buf, fp)) {
	line++;
	if (*buf == '#' || *buf == '\n')
	    continue;
	for (s = buf, i = 0; i <= 1; i++) {
	    if (*s == '1')
		flag[i] = 1;
	    else if (*s == '0')
		flag[i] = 0;
	    else
		flag[i] = -1;
	    if (*++s != ' ' && *s != '\t')
		flag[i] = -1;
	    if (flag[i] < 0) {
		fprintf(stderr, "Invalid flag syntax on line %d of wildtest.txt:%s\n",
			line, buf);
		exit(1);
	    }
	    while (*++s == ' ' || *s == '\t') {}
	}
	for (i = 0; i <= 1; i++) {
	    if (*s == '\'' || *s == '"' || *s == '`') {
		char quote = *s++;
		string[i] = s;
		while (*s && *s != quote) s++;
		if (!*s) {
		    fprintf(stderr, "Unmatched quote on line %d of wildtest.txt:%s\n",
			    line, buf);
		    exit(1);
		}
		end[i] = s;
	    }
	    else {
		if (!*s || *s == '\n') {
		    fprintf(stderr, "Not enough strings on line %d of wildtest.txt:%s\n",
			    line, buf);
		    exit(1);
		}
		string[i] = s;
		while (*++s && *s != ' ' && *s != '\t' && *s != '\n') {}
		end[i] = s;
	    }
	    while (*++s == ' ' || *s == '\t') {}
	}
	*end[0] = *end[1] = '\0';
	run_test(line, flag[0], flag[1], string[0], string[1]);
    }

    if (!wildmatch_errors)
	fputs("No", stdout);
    else
	printf("%d", wildmatch_errors);
    printf(" wildmatch error%s found.\n", wildmatch_errors == 1? "" : "s");

#ifdef COMPARE_WITH_FNMATCH
    if (!fnmatch_errors)
	fputs("No", stdout);
    else
	printf("%d", fnmatch_errors);
    printf(" fnmatch error%s found.\n", fnmatch_errors == 1? "" : "s");

#endif

    return 0;
}
