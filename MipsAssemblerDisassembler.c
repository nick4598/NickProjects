#include "hw1.h"
#include "stdlib.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/*
 * You may modify this file and/or move the functions contained here
 * to other source files (except for main.c) as you wish.
 */
int validateBaseAddress(char *address);
int isSameString(char * str1, char *str2);
int flipEndianness(int numToFlip);


int flipEndianness(int numToFlip)
{
    int p1, p2, p3, p4;
    p1 = numToFlip & (0xFF000000);
    p2 = numToFlip & (0x00FF0000);
    p3 = numToFlip & (0x0000FF00);
    p4 = numToFlip & (0x000000FF);
    p1 = p1 >> 24u;
    p2 = p2 >> 8u;
    p3 = p3 << 8u;
    p4 = p4 << 24u;
    int number = p1 | p2 | p3 | p4;
    return number;

}


/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 1 if validation succeeds and 0 if validation fails.
 * Upon successful return, the selected program options will be set in the
 * global variable "global_options", where they will be accessible
 * elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 1 if validation succeeds and 0 if validation fails.
 * Refer to the homework document for the effects of this function on
 * global variables.
 * @modifies global variable "global_options" to contain a bitmap representing
 * the selected options.
 */
int validargs(int argc, char **argv)
{
    //printf("This should print");
    if (argc == 0)
        return 0;
    if (argc == 1)
        return 0;
   // int hExists = 0;
    int dExists = 0;
    int eExists = 0;
    int aExists = 0;
    int bExists = 0;
    int bigExists = 0;
    int i;
    char *baseAddress;
    for (i = 1; i < argc; i++)
    {
        if (i == 1)
        {
            if (argv[i][0] == '-' && argv[i][1] == 'h' && argv[i][2] == '\0')
            {
               // hExists = 1;
                //set global_options to 0x1
                global_options = 1;
                return 1;
            }
        }
        if (argv[i][0] == '-' && argv[i][1] == 'h' && argv[i][2] == '\0')
            {
                if (i != 1)
                {
                    return 0;
                }
            }
        if (argv[i][0] == '-' && argv[i][1] == 'a' && argv[i][2] == '\0')
        {
            if (eExists || bExists)
            {

                return 0;
            }
            if (dExists)
            {
                return 0;
            }
            aExists = 1;

        }
        if (argv[i][0] == '-' && argv[i][1] == 'd' && argv[i][2] == '\0')
        {
            if (eExists || bExists)
            {
                return 0;
            }
            if (aExists)
            {
                return 0;
            }
            dExists = 1;
        }
        if (argv[i][0] == '-' && argv[i][1] == 'e' && argv[i][2] == '\0')
        {
            eExists = 1;
            if (i == (argc - 1))
            {
                return 0;
            }
            else
            {
                if (argv[i+1][0] == 'b' && argv[i+1][1] == '\0')
                {
                    bigExists = 1;//set the third least significant bit of global_options to 1

                }
                else if (argv[i+1][0] == 'l' && argv[i+1][1] == '\0')
                {

                }
                else
                {
                    return 0;
                }
            }
            if (dExists)
            {
                //set global_options to 0x6
            }

        }
        if (argv[i][0] == '-' && argv[i][1] == 'b' && argv[i][2] == '\0')
        {
            //printf("i = %d", i);
            if (i == (argc - 1))
            {
                //printf("This code was reached");
                return 0;
            }
            else
            {
               if(validateBaseAddress(argv[i+1]))
               {
                    //printf("validateBaseAddresscode was reached");
                    baseAddress = argv[i+1];
                    bExists = 1;
               }
               else
               {
                    //printf("failed validate test");

                    return 0;

               }
            }
        }
    }

        global_options = 0;
        if (dExists)
        {
            global_options = global_options | 2;
        }
        if (eExists)
        {
            if (bigExists)
            {
                global_options = global_options | 4;
            }
        }
        if (bExists)
        {
            long s = strtol(baseAddress, NULL, 16);
            global_options = global_options | s;

        }
        return 1;
}
int isSameString(char *str1, char *str2)
{
    char c = str1[0];
    char c1 = str2[0];
    int i = 1;
    while (c != '\0' && c1 != '\0')
    {
        c = str1[i];
        c1 = str2[i];
        if (c != c1)
        {
            return 0;
        }
        i++;
    }
    if (c == '\0' && c1 == '\0')
    {
        return 1;
    }
    else
    {
        //printf("one wasn't null terminated");
        return 0;
    }
}
int validateBaseAddress(char * address)
{
    char c = 1;
    int length = 0;
    int i = 0;
    //printf("Address = %s", address);
    while (c != '\0')
    {
        c = address[i];
        if (c == '\0')
        {
            break;
        }
        length++;
        if (((c >= '0' && c <= '9') || (c >= 'a' && address[i] <= 'f')) || (address[i] >= 'A' && address[i] <= 'F'))
        {

        }
        else
        {
            //printf("Failed digit checks with digit: %d", address[i]);
            return 0;
        }
        i++;

    }
    if (length > 8)
    {
        //printf("Length > 8");
        return 0;
    }
    if (length <= 3)
    {
       // printf("Length <= 3");
        long s = strtol(address, NULL, 16);
        if (s != 0)
        {
            return 0;
        }
    }
    i = length - 1;
    while (i > length - 3)
    {
        if (address[i] != '0')
        {
          //  printf("Failed address[i] != 0 check");
            return 0;
        }

        i--;
    }
    return 1;


}

/**
 * @brief Computes the binary code for a MIPS machine instruction.
 * @details This function takes a pointer to an Instruction structure
 * that contains information defining a MIPS machine instruction and
 * computes the binary code for that instruction.  The code is returne
 * in the "value" field of the Instruction structure.
 *
 * @param ip The Instruction structure containing information about the
 * instruction, except for the "value" field.
 * @param addr Address at which the instruction is to appear in memory.
 * The address is used to compute the PC-relative offsets used in branch
 * instructions.
 * @return 1 if the instruction was successfully encoded, 0 otherwise.
 * @modifies the "value" field of the Instruction structure to contain the
 * binary code for the instruction.
 */
int encode(Instruction *ip, unsigned int addr) {

    //printf("\nmade it here\n");
    int valueToEncode = 0;
    Instruction instrCopy = *ip;
     //printf("\nmade it here\n");
    Instr_info instrInfo = *(instrCopy.info);
    //printf("\nformatinEncode = %s\n", instrInfo.format);
     //printf("\nmade it here\n");
    if (instrInfo.type == NTYP)
    {
        return 0;
    }
    Opcode op = instrInfo.opcode;
    int foundBCOND = 0;
    int opValue;
    //printf("op = %u", op);
    if (op == ILLEGL)
    {
        return 0;
        //printf("\nOp == illegl\n");
    }
    if (op == OP_BLTZ)
    {
        opValue = 0b00000100000000000000000000000000;
        foundBCOND = 1;
    }
    if (op == OP_BGEZ)
    {
        opValue = 0b00000100000000010000000000000000;
        foundBCOND = 1;
        //printf("made it here");
    }
    if (op == OP_BLTZAL)
    {

        opValue = 0b00000100000100000000000000000000;
        foundBCOND = 1;
    }
    if (op == OP_BGEZAL)
    {
        opValue = 0b00000100000100010000000000000000;
        foundBCOND = 1;
    }

    int indexOfOpcode;
    int foundOpcode = 0;
    int foundSpecialOpcode = 0;
    if (foundBCOND == 0)
    {
        //printf("\nfoundBCOND == 0\n");
        for (int i = 0; i < 64; i++)
        {
            if (op == opcodeTable[i])
            {
                indexOfOpcode = i;
                foundOpcode = 1;
            }
            if (op == specialTable[i])
            {
                indexOfOpcode = i;
                foundSpecialOpcode = 1;
            }

        }
        //printf("indexOfOpcode = %d\n", indexOfOpcode);
        if (foundOpcode == 0 && foundSpecialOpcode == 0)
        {
            return 0;
        }
        if (foundSpecialOpcode == 1)
        {
            opValue = indexOfOpcode;
        }
        else if(foundOpcode == 1)
        {
            opValue = indexOfOpcode << 26;

        }



    }
    valueToEncode = valueToEncode | opValue;
    for (int i = 0; i < 3; i++)
    {
        Source s = instrInfo.srcs[i];
        if (s == RS)
        {
            unsigned int v = instrCopy.args[i];
            //printf("rs value = %d\n", v);
            v = v << 21;

            valueToEncode = valueToEncode | v;

        }
        if (s == RT)
        {
            unsigned int v = instrCopy.args[i];

            //printf("rt value = %d\n", v);
            v = v << 16;
            valueToEncode = valueToEncode | v;
        }
        if (s == RD)
        {
            unsigned int v = instrCopy.args[i];
            //printf("rd value = %d\n", v);
            v = v << 11;
            valueToEncode = valueToEncode | v;
        }
        if (s == EXTRA)
        {
            if (op == OP_BREAK)
            {
                unsigned int v = instrCopy.args[i];
                v = v << 6;
                valueToEncode = valueToEncode | v;
            }
            else if (instrInfo.type == RTYP)
            {
                unsigned int v = instrCopy.args[i] << 6;
                valueToEncode = valueToEncode | v;
            }
            else if (instrInfo.type == ITYP)
            {
                int v = instrCopy.args[i];
                //printf("Extra = %d\n", v);
               // v = v << 16;
                   // v = v >> 16u;
                if ((op == OP_BEQ) || (op == OP_BGEZ) || (op == OP_BGEZAL) || (op == OP_BGTZ) ||
                    (op == OP_BLEZ) || (op == OP_BLTZ) || (op == OP_BLTZAL) || (op == OP_BNE))
                {
                    v = v - addr - 4;
                    v = v >> 2u;
                    v = v & (0x0000FFFF);

                    valueToEncode = valueToEncode | v;

                }
                else
                {
                    v = v & (0x0000FFFF);
                    valueToEncode = valueToEncode | v;
                }

            }
            else if (instrInfo.type == JTYP) // if j type instructiosn dont work this needs to be changed
            {

                unsigned int v = instrCopy.args[i] << 4;
                v = v >> 4;
                v = v >> 2;
                valueToEncode = valueToEncode | v;

            }
        }
    }
    (*ip).value = valueToEncode;
    return 1;
}

/**
 * @brief Decodes the binary code for a MIPS machine instruction.
 * @details This function takes a pointer to an Instruction structure
 * whose "value" field has been initialized to the binary code for
 * MIPS machine instruction and it decodes the instruction to obtain
 * details about the type of instruction and its arguments.
 * The decoded information is returned by setting the other fields
 * of the Instruction structure.
 *
 * @param ip The Instruction structure containing the binary code for
 * a MIPS instruction in its "value" field.
 * @param addr Address at which the instruction appears in memory.
 * The address is used to compute absolute branch addresses from the
 * the PC-relative offsets that occur in the instruction.
 * @return 1 if the instruction was successfully decoded, 0 otherwise.
 * @modifies the fields other than the "value" field to contain the
 * decoded information about the instruction.
 */
/*
Our first objective in decoding an instruction is to determine the proper
Opcode value, thereby obtaining access to the proper entry from the instruction table.
*/

int decode(Instruction *ip, unsigned int addr) {

    Instruction instrCopy = *ip;
     //printf("\nmade it hereeeeeee\n");
    unsigned int placeHolder;
     //printf("\nmade it hereeeeeee\n");
    placeHolder = instrCopy.value & 0b11111100000000000000000000000000;
    //printf("\n%u\n", placeHolder);
     //printf("\nmade it hereeeeeee\n");
    placeHolder = placeHolder >> 26;
     //printf("\nmade it hereeeeeee\n");
    //printf("\n%u\n", placeHolder);
    Opcode op = opcodeTable[placeHolder];
     //printf("\nmade it hereeeeeee\n");
   //  printf("\nmade it hereeeeeee\n");
    for (int i = 0; i < 3; i++)
    {
        instrCopy.regs[i] = 0;
        instrCopy.args[i] = 0;
    }
    if (op == ILLEGL)
    {
        return 0;
    }
    if (op == SPECIAL)
    {
        placeHolder = instrCopy.value & (0b00000000000000000000000000111111);
        //placeHolder = placeHolder >> 26;
        op = specialTable[placeHolder];
        if (op == ILLEGL)
        {
            return 0;
        }
    }
    else if (op == BCOND)
    {
        placeHolder = instrCopy.value & (0b00000000000111110000000000000000);
        placeHolder = placeHolder >> 16;
        if (placeHolder == 0b00000)
        {
            op = OP_BLTZ;
        }
        else if (placeHolder == 0b00001)
        {
            op = OP_BGEZ;
        }
        else if (placeHolder == 0b10000)
        {
            op = OP_BLTZAL;
        }
        else if (placeHolder == 0b10001)
        {
            op = OP_BGEZAL;
        }
        else
        {
            return 0;
        }
    }
    Instr_info instrInfo = instrTable[op];
    Instr_info *ptrToInstrInfo = &instrInfo;
    (*ip).info = ptrToInstrInfo;
    //printf("format = %s", instrInfo.format);
    for (int i = 0; i < 3; i++)
    {
        //printf("made it hereeeeeee");
        Source s = instrInfo.srcs[i];
        if (s ==RS)
        {
            placeHolder = (*ip).value & (0b00000011111000000000000000000000);
            placeHolder = placeHolder >> 21; //26:0 need to be removed
            (*ip).regs[0] = placeHolder;
            (*ip).args[i] = placeHolder;
            //printf("made it hereeeeeee");
        }
        else if (s ==RT)
        {
            placeHolder = (*ip).value & (0b00000000000111110000000000000000);
            placeHolder = placeHolder >> 16;
            (*ip).regs[1] = placeHolder;
            (*ip).args[i] = placeHolder;
            //printf("made it hereeeeeee");
        }
        else if (s == RD)
        {
            placeHolder = (*ip).value & (0b00000000000000001111100000000000);
            placeHolder = placeHolder >> 11;
            (*ip).regs[2] = placeHolder;
            (*ip).args[i] = placeHolder;
            //printf("made it hereeeeeee");

        }
        else if (s == NSRC)
        {
            (*ip).args[i] = 0;
            //instrCopy.regs[i] = 0; // is this line right
            //printf("made it hereeeeeee");
        }
        else if (s == EXTRA)
        {
            if (op == OP_BREAK)
            {
                placeHolder = (*ip).value & (0b00000011111111111111111111000000); // want bits 31:12
                placeHolder = placeHolder >> 6;
                //instrCopy.regs[i] = 0; //regs[i] shouldnt be set to place holder
                (*ip).args[i] = placeHolder;
                (*ip).extra = placeHolder;
            }
            else if (instrInfo.type == RTYP)
            {

                placeHolder = (*ip).value & (0b00000000000000000000011111000000);
                placeHolder = placeHolder >> 6;
                //instrCopy.regs[i] = 0;
                (*ip).args[i] = placeHolder;
                (*ip).extra = placeHolder;
            }
            else if (instrInfo.type == ITYP)
            {
                placeHolder = (0x0000FFFF & (*ip).value);
                int sign = placeHolder >> 15;
                if (sign == 1)
                {
                    placeHolder = placeHolder | 0xFFFF0000;
                }

                if (op == OP_BEQ || op == OP_BGEZ || op == OP_BGEZAL || op == OP_BGTZ ||
                op == OP_BLEZ || op == OP_BLTZ || op == OP_BLTZAL || op == OP_BNE)
                {
                    placeHolder = placeHolder << 2;
                    (*ip).args[i] = placeHolder + addr + 4;
                    //instrCopy.regs[i] = NSRC; // idk tihs is wrong probably
                   // instrCopy.regs[i] = 0;
                    (*ip).extra = placeHolder + addr + 4;

                }
                else
                {
                    (*ip).args[i] = placeHolder;
                    //instrCopy.regs[i] = 0; // wrong probably idk
                    (*ip).extra = placeHolder;
                }
            }
            else if (instrInfo.type == JTYP)
            {
                unsigned int pH = (*ip).value & (0b00000011111111111111111111111111);
                pH = pH << 2;
                int programCounter = addr + 4;
                programCounter = programCounter & 0xF0000000; // zeroing the 28 lsb
                programCounter += pH;
                //instrCopy.regs[i] = 0;
                (*ip).args[i] = programCounter;
                (*ip).extra = programCounter;

            }

            }
        }
        return 1;
    }





