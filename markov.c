#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

enum
{
    NPREF	= 1,
    NHASH	= 4093,
};

typedef struct State State;
typedef struct Suffix Suffix;

struct State
{
    char	*pref[NPREF];
    Suffix	*suf;
    State	*next;
};

struct Suffix
{
    char	*word;
    Suffix	*next;
};

State	*search(char *prefix[], int create);
void	build_table(char *prefix[], FILE*);
void	text_gen(unsigned int nwords, char* startWord);
void	add_pref(char *prefix[], char *word);
void    add_suf(State *sp, char *suffix);
void WriteTable(char *prefix[NPREF], FILE *f);
void ReadTable(FILE *f);


State	*state_table[NHASH];

char NONWORD[] = "\n";

int main(int argc, char* argv[])
{
    int i, nwords;
    char *prefix[NPREF];

    int c;


        int action;
        int action1;
        char finame[15];

    char startWord[32];

        for(i = 0; i < 32; i++)
           startWord[i] = '\0';


        long seed = time(NULL);

    srand(seed);
    for (i = 0; i < NPREF; i++)
        prefix[i] = NONWORD;

    if (argc < 4)
    {
        printf("Error!\n");
        exit(-1);
    }

    char* whatDoing = argv[1];
    char* statName = argv[2];

    if (strcmp(whatDoing, "learn") == 0)
    {

        for (int i = 3; i < argc; ++i)
        {
            char* fileName = argv[i];

            FILE *f = fopen( fileName, "r" );
            if ( !f )
            {
                printf("Файл не найден!\n");
                exit(-1);
            }

            build_table(prefix, f);
            add_pref(prefix, NONWORD);
            fclose(f);
        }

        FILE *fWrite = fopen(statName, "w+");
        if (fWrite == NULL)
        {
            printf("Error!\n");
            exit(-1);
        }
        WriteTable(prefix, fWrite);

    }
    else if (strcmp(whatDoing, "gen") == 0)
    {

        unsigned int countWords = atoi(argv[3]);
        char* startWord = NULL;

        FILE *fRead = fopen(statName, "r");
        if (fRead == NULL)
        {
            printf("Error!\n");
            exit(-1);
        }
        ReadTable(fRead);

        if (argc == 5)
        {
            startWord = argv[4];
            text_gen(countWords - 1, startWord);
        }
        else
        {
            startWord = NONWORD;
            text_gen(countWords, startWord);
        }

        printf("\n");
    }

    return 0;
}

const int MLP = 31;

unsigned int hash(char *s[NPREF])
{
    unsigned int h;
    unsigned char *p;
    int i;

    h = 0;
    for (i = 0; i < NPREF; i++)
        for (p = (unsigned char *) s[i]; *p != '\0'; p++)
            h = MLP * h + *p;
    return h % NHASH;
}


void WriteTable(char *prefix[NPREF], FILE *f)
{
    State *sp;
    Suffix *ss;

    for (int j = 0; j < NHASH; ++j)
    {
        if (state_table[j] == NULL)
            continue;
        fwrite(&j, sizeof(int), 1, f);
        int cntSp = 0;
        for (sp = state_table[j]; sp != NULL; sp = sp->next)
            cntSp++;
        fwrite(&cntSp, sizeof(int), 1, f);
        for (sp = state_table[j]; sp != NULL; sp = sp->next)
        {
            for (int i = 0; i < NPREF; ++i)
            {
                int sz = strlen(sp->pref[i]);
                fwrite(&sz, sizeof(int), 1, f);
                fwrite(sp->pref[i], sizeof(char), sz, f);
            }
            int cntSuf = 0;
            for (ss = sp->suf; ss != NULL; ss = ss->next)
                cntSuf++;
            fwrite(&cntSuf, sizeof(int), 1, f);
            for (ss = sp->suf; ss != NULL; ss = ss->next)
            {
                int sz = strlen(ss->word);
                fwrite(&sz, sizeof(int), 1, f);
                fwrite(ss->word, sizeof(char), sz, f);
            }

        }

    }
    fclose(f);
}

void ReadTable(FILE *f)
{
    State *total;
    Suffix *ss;


    int pos;

    while (1)
    {
        State *genSp;
        int sz;
        int cntReads = fread(&pos, sizeof(int), 1, f);
        if (!cntReads)
            break;
        int cntSp;
        fread(&cntSp, sizeof(int), 1, f);
        for (int i = 0; i < cntSp; ++i)
        {
            State *sp = (State*) malloc(sizeof(State));
            for (int k = 0; k < NPREF; ++k)
            {
                int sz;
                fread(&sz, sizeof(int), 1, f);
                sp->pref[k] = (char*) malloc(sizeof(char) * sz);
                fread(sp->pref[k], sizeof(char), sz, f);
            }
            int cntSuf;
            fread(&cntSuf, sizeof(int), 1, f);
            for (int k = 0; k < cntSuf; ++k)
            {
                Suffix *tmpSuf = (Suffix*) malloc(sizeof(Suffix));
                int sz;
                fread(&sz, sizeof(int), 1, f);
                tmpSuf->word = (char*) malloc(sizeof(char) * sz);
                fread(tmpSuf->word, sizeof(char), sz, f);
                if (!k)
                    sp->suf = ss = tmpSuf;
                else
                {
                    ss->next = tmpSuf;
                    ss = ss->next;
                }
            }
            if (!i)
                total = genSp = sp;
            else
            {
                genSp->next = sp;
                genSp = genSp->next;
            }
        }
        state_table[pos] = total;
    }
    fclose(f);
}


State* search(char *prefix[NPREF], int create)
{
    unsigned int i, h;
    State *sp;

    h = hash(prefix);

    for (sp = state_table[h]; sp != NULL; sp = sp->next)
    {
        for (i = 0; i < NPREF; i++)
            if (strcmp(prefix[i], sp->pref[i]) != 0)
                break;
        if (i == NPREF)
        {
            return sp;
        }
    }

    if (create)
    {
        sp = (State *) malloc(sizeof(State));
        for (i = 0; i < NPREF; i++)
            sp->pref[i] = prefix[i];
        sp->suf = NULL;
        sp->next = state_table[h];
        state_table[h] = sp;
    }

    return sp;
}


void add_suf(State *sp, char *suffix)
{
    Suffix *suf;

    suf = (Suffix *) malloc(sizeof(Suffix));
    suf->word = suffix;
    suf->next = sp->suf;
    sp->suf = suf;
}


void add_pref(char *prefix[NPREF], char *suffix)
{
    State *sp;

    sp = search(prefix, 1);
    add_suf(sp, suffix);

    memmove(prefix, prefix+1, (NPREF-1)*sizeof(prefix[0]));
    prefix[NPREF-1] = suffix;
}


void build_table(char *prefix[NPREF], FILE *f)
{
    char buf[100], fmt[10];

    sprintf(fmt, "%%%ds", sizeof(buf)-1);
    while (fscanf(f, fmt, buf) != EOF)
    {

        add_pref(prefix, strdup(buf));
    }
}


void text_gen(unsigned int nwords, char* startWord)
{
    State *sp;
    Suffix *suf;
    char *prefix[NPREF], *w;
    int i, nmatch;

    for (i = 0; i < NPREF; i++)
        prefix[i] = startWord;

    if (strcmp(startWord, NONWORD) != 0)
        printf("%s ", startWord);

    for (i = 0; i < nwords; i++)
    {
        sp = search(prefix, 0);

        //startWord = NULL;
        if (sp == NULL)
            continue;

        nmatch = 0;
        for (suf = sp->suf; suf != NULL; suf = suf->next)
        {
            if (rand() % ++nmatch == 0)
                w = suf->word;
        }
        if (nmatch == 0)
            printf("internal error: no suffix %d %s", i, prefix[0]);
        if (strcmp(w, NONWORD) == 0)
            break;
        printf("%s ", w);
        memmove(prefix, prefix+1, (NPREF-1)*sizeof(prefix[0]));
        prefix[NPREF-1] = w;
    }
}
