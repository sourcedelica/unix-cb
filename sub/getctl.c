#include <stdio.h>
#include <string.h>
#include <malloc.h>

/*
 *	rdctl(filename)-
 *		reads 'filename' and returns a char ** to it's contents
 *
 *	wrctl(p, filename)-
 *		the complement of rdctl.  p is the char ** to write.
 *
 *	getctl(name, p)-
 *		similar to getenv, except it searches through char **p for
 *		'name' and returns a char * to the string after the "=".
 *
 *	putctl(s, p)-
 *		similar to putenv, except it adds s to char **p.  s is in
 *		the form: "NAME=value" (no nl).
 *
 *	clrctl(s, p)-
 *		removes char *s from char **p.  assumes environment type
 *		char **p.
 *
 *	freectl(p)-
 *		frees the mem alloc'd for p
 */

char **rdctl(fname)
char *fname;
{
	static	char	line[800];
	char	**p, *x;
	FILE	*fp;
	int	i;

	if ((fp=fopen(fname, "r")) == NULL)
		return ( (char **) NULL );
	p = (char **) malloc(sizeof(char **) );		/* char *'s to lines */

	for (i=0; fgets(line, 255, fp) ;i++) {
		if( *line == '#' || *line == '\n' ){
			i--;
			continue;
		}
		x = strtok(line, "\n");
		p[i] = strdup(x);
		p = (char **) realloc(p, (i+2)*sizeof(char *) );
	}
	p[i] = NULL;
	fclose(fp);
	return( p );
}

char **wrctl(p, fname)
char **p, *fname;
{
	char	line[256];
	FILE	*fp;
	int	i;

	if ((fp=fopen(fname, "w")) == NULL)
		return ( NULL );
	if( p != (char **)NULL )
		for (i=0; p[i] != NULL ; i++)
			fprintf(fp,"%s\n",p[i]);
	fclose(fp);
	return( p );
}

void freectl(p)
char **p;
{
	int	i;

	for (i=0; p[i] != NULL ;i++)
		free(p[i]);

	free(p);
}

char *getctl(name, p)
char **p, *name;
{
	int	i;
	char	*x, *y, *ret;
	static char buf[512];

	if( p == (char **)NULL )
		return( NULL );

	for (i=0; p[i] != NULL ;i++) {
		x = strcpy( buf, p[i] );
		y = strtok(x, "=");

		if (!strcmp(y, name) ) {
			if (p[i][strlen(y)] == '=') {
				if (p[i][strlen(y)+1] == 0)
					ret = "";
				else
					ret = p[i]+strlen(y)+1;
			} else
				ret = "";
			return( ret );
		}
	}
	return( NULL );
}

char **putctl(line, p)
char **p, *line;
{
	int	i;
	char	*x, *y;

	if( p == (char **)NULL ){
		p = (char **)malloc( 2*sizeof(char **) );
		p[0] = strdup(line);
		p[1] = NULL;
		return( p );
	}
		
	for (i=0; p[i] != NULL ;i++) {
		x = strdup(p[i]);
		y = strdup(line);

		if (!strcmp(strtok(x,"="), strtok(y, "=")) ) {
			free(p[i]);
			p[i] = strdup(line);
			return( p );
		}
	}
	p = (char **) realloc(p, (i+2)*sizeof(char *) );
	p[i] = strdup(line);
	p[++i] = NULL;
	return( p );
}

char **clrctl(line, p)
char	**p, *line;
{
	int	i;
	char	*x, *y;

	for (i=0; p[i] != NULL; i++) {
		x = strdup(p[i]);
		y = strdup(line);

		if (!strcmp(strtok(x,"="), strtok(y, "=")) ) {
			free(p[i]);
			for (; p[i] != NULL; i++) 
				p[i] = p[i+1];
			p[i] = NULL;
			return( p );
		}
	}
}
