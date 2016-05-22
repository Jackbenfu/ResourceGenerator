//
//  resourceGenerator.c
//  Resource generator
//
//  Created by Damien Bendejacq on 21/05/16.
//  Copyright (c) 2016 Damien Bendejacq. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MINIMUM_ARG_COUNT           3
#define OUTPUT_FILE_ARG_INDEX       1
#define FIRST_RESOURCE_ARG_INDEX    2

#define READ_FILE_BUFFER_LENGTH     256
#define OUTPUT_FILE_COLUMN_SIZE     10

#define NON_ALPHANUM_REPLACEMENT    '_'

void usage()
{
    fprintf(
        stderr,
        "Usage: resourceGenerator {outputFile} {resource1} [{resource2}] ... [{resourceN}]\n"
    );
}

FILE* openFile(const char *name, const char *mode)
{
    FILE *file = fopen(name, mode);
    if (NULL == file)
    {
        perror(name);
    }
    return file;
}

FILE* prepareOutputFile(const char *name)
{
    return openFile(name, "w");
}

FILE* prepareResourceFile(const char *name)
{
    return openFile(name, "r");
}

int appendResource(FILE *outputFile, char *resourceName)
{
    FILE *resourceFile = prepareResourceFile(resourceName);
    if (NULL == resourceFile)
    {
        fclose(outputFile);
        return -1;
    }

    int resourceNameLength = (int)strlen(resourceName);
    int resourceNameIndex = resourceNameLength - 1;
    while (resourceNameIndex > -1)
    {
        if (resourceName[resourceNameIndex] == '/' ||
            resourceName[resourceNameIndex] == '\\')
        {
            break;
        }

        if (!isalnum(resourceName[resourceNameIndex]))
        {
            resourceName[resourceNameIndex] = NON_ALPHANUM_REPLACEMENT;
        }

        --resourceNameIndex;
    }
    ++resourceNameIndex;

    char resourceVariableName[FILENAME_MAX];
    memset(resourceVariableName, 0, FILENAME_MAX);
    memcpy(
        resourceVariableName,
        resourceName + resourceNameIndex,
        resourceNameLength - resourceNameIndex
    );

    fprintf(outputFile, "\n");
    fprintf(
        outputFile,
        "const unsigned char %s[] = {\n",
        resourceVariableName
    );

    unsigned char buf[READ_FILE_BUFFER_LENGTH];
    size_t nRead = 0;
    size_t lineCount = 0;
    do
    {
        nRead = fread(buf, 1, sizeof(buf), resourceFile);
        size_t i;
        for (i = 0; i < nRead; ++i)
        {
            if (0 == lineCount)
            {
                fprintf(outputFile, "    ");
            }

            fprintf(outputFile, "0x%02x, ", buf[i]);
            if (++lineCount == OUTPUT_FILE_COLUMN_SIZE)
            {
                fprintf(outputFile, "\n");
                lineCount = 0;
            }
        }
    }
    while (nRead > 0);

    fprintf(outputFile, "};\n");
    fprintf(
        outputFile,
        "const size_t %s_size = sizeof(%s);\n",
        resourceVariableName,
        resourceVariableName
    );

    fclose(resourceFile);

    return 0;
}

int main(int argc, char ** argv)
{
    if (argc < MINIMUM_ARG_COUNT)
    {
        usage();
        return EXIT_FAILURE;
    }

    FILE *outputFile = prepareOutputFile(argv[OUTPUT_FILE_ARG_INDEX]);
    if (NULL == outputFile)
    {
        return EXIT_FAILURE;
    }

    fprintf(outputFile, "#include <stdlib.h>\n");
    for (int resourceIndex = FIRST_RESOURCE_ARG_INDEX; resourceIndex < argc; ++resourceIndex)
    {
        if (-1 == appendResource(outputFile, argv[resourceIndex]))
        {
            break;
        }
    }

    fclose(outputFile);

    return EXIT_SUCCESS;
}
