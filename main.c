#include <stdlib.h>

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

#include "hw1.h"
#include "debug.h"

int main(int argc, char **argv)
{
    if(!validargs(argc, argv))
        USAGE(*argv, EXIT_FAILURE);
    debug("Options: 0x%X", global_options);
    if(global_options & 0x1) {
        USAGE(*argv, EXIT_SUCCESS);
    }

    unsigned int addr = global_options & 0xFFFFF000;
    //printf("address = %d", addr);
    //printf("GLobal_options = %d", global_options);
    //printf("\nbefore checking global options");
    fflush(stdout);
    //char *stt = "\nbefore checking global options";
    fflush(stdout);
    //printf("%s", stt);


    if (global_options & 2) // this means disassembly mode
    {
        //printf("it understands that its in disassembly");
        //int decodeNumber = 0;
        //Instruction testInstr;
        //testInstr.value = 0x8cc50007;
        //decode(&testInstr, 0);
        //Instr_info testInstrInfo = *(testInstr.info);
        //printf(testInstrInfo.format, testInstr.args[0], testInstr.args[1], testInstr.args[2]);

        while (1)
        {
            //printf("made it hereeeeeee");
            //int * decodeNumber = malloc(sizeof(int));
            //printf("made it here\n");
            int decodeNumber = 0;
            int success = fread(&decodeNumber, sizeof(int), 1, stdin);
            //printf("success = %d", success);
            //printf("made it here\n");
            if (feof(stdin))
            {
                //printf("made it hereeeeeee");
                break;
            }
            if (success == 0)
            {
                //printf("made it hereeeeeee");
                return EXIT_FAILURE;
            }

            Instruction ip;
            ip.value = decodeNumber;
            if (global_options & 4)
            {
                decodeNumber = flipEndianness(decodeNumber);
            }
            //printf("made it hereeeeeee");
            decode(&ip, addr);
            addr = addr + 4;
            Instr_info instrInfo = *(ip.info);
            printf(instrInfo.format, ip.args[0], ip.args[1], ip.args[2]);
            printf("\n");
            //printf("made it here\n");
            fflush(stdin);

        }
       // printf("made it hereeeeeee");



    }
    else // this means assembly mode
    {
        //printf("\nmade it here\n");
        int passedEncode = 0;
        Instruction ip;
        ip.regs[0] = 0;
        ip.regs[1] = 0;
        ip.regs[2] = 0;
        ip.args[0] = 0;
        ip.args[1] = 0;
        ip.args[2] = 0;
        char str[120];
        int lengthOfStrArray = 0;
        //printf("\nmade it here\n");
        while (1)
        {
            //printf("\nmade it here\n");
            if (feof(stdin))
            {
                break;
            }
            //printf("\nmade it here\n");
            fgets(str, 120, stdin);
            //printf("\nmade it here\n");
            //printf("input = %s", str);
            //printf("matches = %d", isSameString(str, "syscall"));
            int numbers[3];
            numbers[0] = 0;
            numbers[1] = 0;
            numbers[2] = 0;
            //int *numbers0 = &numbers[0];
            //int *numbers1 = &numbers[1];
            //int *numbers2 = &numbers[2];
            //printf("\nmade it here\n");
            for(int j = 0; j < 64; j++)
            {
                //printf("\nmade it here\n");
                Instr_info instrInfo = instrTable[j];
                //printf("\nmade it here\n");

                char *format = instrInfo.format;
                //printf("\nmade it here\n");printf("format = %s", format);
                //printf("format = %s\n", format);
                //printf("str = %s\n", str);
                //printf("format = %s\n", format);

                int numbersMatched = sscanf(str, format, &(numbers[0]), &(numbers[1]),&(numbers[2]));
                //printf("\nmade it here\n");
                if (numbersMatched == 0 && (isSameString(str, "ILLEGL\n") ||
                    isSameString(str, "rfe\n") || isSameString(str, "syscall\n")
                    || isSameString(str, "Unimplemented\n")))
               {
                     //printf("\nmade it here\n");
                    ip.info = &instrTable[j];
                    ip.regs[0] = 0;
                    ip.regs[1] = 0;
                    ip.regs[2] = 0;
                    ip.extra = 0;
                    ip.args[0] = 0;
                    ip.args[1] = 0;
                    ip.args[2] = 0;
                    encode(&ip, addr);
                    if (global_options & 4)
                    {
                        int flipped = flipEndianness(ip.value);
                        ip.value = flipped;
                    }
                    addr = addr + 4;
                    if (!feof(stdin))
                    {
                        //printf("\nmade it here\n");
                         fwrite(&(ip.value), sizeof(int), 1, stdout);
                    }

                    passedEncode = 1;
                    break;
              }
              else if (numbersMatched > 0)
              {
                 //printf("\nmade it here\n");
                    for (int z = 0; z < 3; z++)
                    {
                        Source s = instrInfo.srcs[z];
                         //printf("\nmade it here\n");
                        if (s == RS)
                        {
                            ip.regs[0] = numbers[z];
                            ip.args[z] = numbers[z];

                        }
                        else if (s == RT)
                        {
                            ip.regs[1] = numbers[z];
                            ip.args[z] = numbers[z];
                        }
                        else if (s == RD)
                        {
                            ip.regs[2] = numbers[z];
                            ip.args[z] = numbers[z];
                        }
                        else if (s == EXTRA)
                        {
                            ip.extra = numbers[z];
                            ip.args[z] = numbers[z];
                        }

                    }
                    //printf("\ninfoFormat = %s\n", instrInfo.format);
                    ip.info = &(instrInfo);
                     //printf("\nmade it here\n");
                    encode(&ip, addr);
                     //printf("\nmade it here\n");
                    addr = addr + 4;
                    if (!feof(stdin))
                    {
                       // printf("\nmade it here\n");

                        fwrite(&(ip.value), sizeof(int), 1, stdout);
                    }

                    passedEncode = 1;
                    break;
              }


            }
            if (passedEncode == 0)
            {
                return EXIT_FAILURE;
            }
        }
           //printf("made it here");


    }

    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
