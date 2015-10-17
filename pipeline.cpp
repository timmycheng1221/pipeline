#include<iostream>
#include<fstream>
#include<string.h>
#include<bitset>
using namespace std;
int main()
{
    string op_ins[100]; //store 32-bit machine code
    int op[100] , op_rs[100] , op_rt[100] , op_rd[100] , op_im[100];
    //store machine code's opcode,rs,rt,rd or immediately
    for(int i = 0 ; i < 100 ; i++) //reset
    {
        op_ins[i].clear();
        op[i] = 0;
        op_rs[i] = 0;
        op_rt[i] = 0;
        op_rd[i] = 0;
        op_im[i] = 0;
    }
    int reg[9] = {0 , 1 , 2 , 3 , 4 , 1 , 6 , 7 , 8}; //reset registers
    int mem_acc[5] = {1 , 2 , 3 , 4 , 5}; //reset memory

    int ctrlsig[4] = {0,0,0,0} , ALUout1[4] = {0,0,0,0} , ALUout2[4] =  {0,0,0,0};
    //reset 4-stage's control signals,ALUout1,ALUout2
    //ALUout1[4] store ALUout , ALUout2[4] store memory read
    int rs[4] = {0,0,0,0} , rt[4] = {0,0,0,0} , rd[4] = {0,0,0,0} , im[4] = {0,0,0,0};
    //reset 4-stage's rs,rt,rd,immediately
    int name[4] =  {0,0,0,0} , CC = 0 , PC = 1 , j = 0;
    //reset 4-stage's instruction number,number,procedure counter and machine code number

    bool stall = false; //reset stall

    string opcode[7] = {"100011" , "101011"  , "000100" , "100000" , "100010" , "100100"
     , "100101"}; //machine code->instruction
    string op_name[7] = {"lw" , "sw" , "beq" , "add" , "sub" , "and" , "or"};
    //instruction number->instruction name
    string ins[4]; //4-stage's instruction name
    char opcode_char[6] , s_char[40];
    //opcode_char[] store opcode,s_char store machine code

    ifstream fin("genSample.txt"); //fin -> read file's text
    ofstream fout("genResult.txt"); //fout ->write in file's text
    string s;
    while(fin >> s) //read file's text until EOF
    {
        op_ins[j] = s; //op_ins[] store machine code
        strcpy(s_char , s.c_str()); //string to char[]
        int bin = 1; //to count binary number
        for(int i = 10 ; i >= 6 ; i--) //count rs
        {
            op_rs[j] += (s_char[i] - '0') * bin;
            bin *= 2;
        }
        bin = 1;
        for(int i = 15 ; i >= 11 ; i--) //count rt
        {
            op_rt[j] += (s_char[i] - '0') * bin;
            bin *= 2;
        }
        if(strncmp(s_char , "000000" , 6) == 0) //R-type
        {
            bin = 1;
            for(int i = 20 ; i >= 16 ; i--) //count rd
            {
                op_rd[j] += (s_char[i] - '0') * bin;
                bin *= 2;
            }
            for(int i = 3 ; i < 7 ; i++)
            {
                strcpy(opcode_char , opcode[i].c_str()); //string to char[]
                if(strncmp(s_char + 26 , opcode_char , 6) == 0) //opcode->instruction number
                {
                    op[j] = i;
                    break;
                }
            }
        }
        else
        {
            bin = 1;
            for(int i = 31 ; i >= 16 ; i--) //count immediately
            {
                op_im[j] += (s_char[i] - '0') * bin;
                bin *= 2;
            }
            for(int i = 0 ; i < 3 ; i++)
            {
                strcpy(opcode_char , opcode[i].c_str()); //string to char[]
                if(strncmp(s_char , opcode_char , 6) == 0) //opcode->instruction number
                {
                    op[j] = i;
                    break;
                }
            }
        }
        j++;
    }
    //store information to 0-stage(IF/ID)//
    ins[0] = op_ins[0];
    name[0] = op[0];
    rs[0] = op_rs[0];
    rt[0] = op_rt[0];
    rd[0] = im[0] = 0;
    if(name[0] < 3) //beq,sw,lw
    im[0] = op_im[0];
    else            //R-type
        rd[0] = op_rd[0];

    //store 0-stage's control signals//
    if(name[0] > 2)       //R-type
        ctrlsig[0] = 386;
    else if(name[0] == 2) //beq
        ctrlsig[0] = 80;
    else if(name[0] == 1) //sw
        ctrlsig[0] = 36;
    else if(name[0] == 0) //lw
        ctrlsig[0] = 43;
    else                  //empty instruction
        ctrlsig[0] = 0;

    bool flag = false;    //cycle continues(false) or ends(true)
    while(true)
    {
        if(flag)
            break;        //cycle ends
        if(ins[0].empty() && ins[1].empty() && ins[2].empty() && ins[3].empty())
            flag = true;  //if every stage's instruction is empty , then cycle ends
        unsigned A , B;   //A = ReadData1 , B = ReadData2
        //forwarding unit//
        bool ForwardA = false , ForwardB = false; //reset forwardingA , forwardingB
        if(ctrlsig[2]&2) //EX/MEM.RegWrite = 1 -> R-type and lw
        {
            if(ctrlsig[2]&1) //lw
            {
                if(rs[1] == rt[2]) //ID/EX.RegisterRs = EX/MEM.RegisterRt
                {
                    ForwardA = true;
                    A = ALUout1[2];
                }
                if(rt[1] == rt[2]) //ID/EX.RegisterRt = EX/MEM.RegisterRt
                {
                    ForwardB = true;
                    B = ALUout1[2];
                }
            }
            else //R-type
            {
                if(rs[1] == rd[2]) //ID/EX.RegisterRs = EX/MEM.RegisterRd
                {
                    ForwardA = true;
                    A = ALUout1[2];
                }
                if(rt[1] == rd[2]) //ID/EX.RegisterRt = EX/MEM.RegisterRd
                {
                    ForwardB = true;
                    B = ALUout1[2];
                }
            }
        }
        if(ctrlsig[3]&2)  //MEM/WB.RegWrite = 1 -> R-type and lw
        {
            if(ctrlsig[3]&1) //lw
            {
                if(rs[1] == rt[3] && !ForwardA) //ID/EX.RegisterRs = MEM/WB.RegisterRt
                {
                    ForwardA = true;
                    A = ALUout2[3];
                }
                if(rt[1] == rt[3] && !ForwardB) //ID/EX.RegisterRt = MEM/WB.RegisterRt
                {
                    ForwardB = true;
                    B = ALUout2[3];
                }
            }
            else //R-type
            {
                if(rs[1] == rd[3] && !ForwardA) //ID/EX.RegisterRs = MEM/WB.RegisterRd
                {
                    ForwardA = true;
                    A = ALUout2[3];
                }
                if(rt[1] == rd[3] && !ForwardB)  //ID/EX.RegisterRt = MEM/WB.RegisterRd
                {
                    ForwardB = true;
                    B = ALUout2[3];
                }
            }
        }
        if(rs[1] == 0)
            ForwardA = false;
        if(rt[1] == 0)
            ForwardB = false;
        if(!ForwardA)   A = reg[rs[1]]; //ReadData1 = ID/EX.RegisterRs value
        //else->forwardingA
        if(!ForwardB)   B = reg[rt[1]]; //ReadData2 = ID/EX.RegisterRt value
        //else->forwardingB

        //forwarding unit//

        fout << "CC " << ++CC << ":\n\n"; //cycle++
        fout << "Registers:";
        for(int i = 0 ; i < 9 ; i++)      //output register number and value
        {
            if(i % 3 == 0) //output "\n"
                fout << "\n";
            fout << "$" << i << ":   " << reg[i] << "   ";
        }
        fout << "\nData memory:\n";       //output memory name and value
        for(int i = 0 ; i < 5 ; i++)
        {
            if(i < 3) //output "0"
                fout << "0";
            fout << i * 4 << ":     " << mem_acc[i] << "\n";
        }
        //***************IF/ID************************//
        fout << "\nIF/ID :\n";
        fout << "PC              " << ((PC + (stall == true) ) * 4) << "\n";
        if(ins[0].empty()) //empty instrution
            fout << "Instruction     00000000000000000000000000000000\n\n";
        else
            fout << "Instruction     " << ins[0] << "\n\n";

        //***************ID/EX************************//
        fout << "ID/EX :\n";
        fout << "ReadData1       " << A;
        fout << "\nReadData2       " << B;
        fout << "\nsign_ext        "; //Immediately
        if(name[1] < 3)
            fout << im[1] << "\n";
        else
            fout << "0\n";
        fout << "Rs              " << rs[1];
        fout << "\nRt              " << rt[1];
        fout << "\nRd              " << rd[1];
        fout << "\nControl signals ";
        bitset<9>s1(ctrlsig[1]); //9-bit control signals
        fout << s1 << "\n\n";

        //***************EX/MEM***********************//
        fout << "EX/MEM :\n";
        fout << "ALUout          " << ALUout1[2];
        fout << "\nWriteData       ";
        if(ctrlsig[2]&4) //sw
            fout << reg[rt[2]]; //output RegisterRt value
        else
            fout << 0;
        if(name[2] < 3) //lw , sw , beq
            fout << "\nRt              " << rt[2]; //output RegisterRt
        else //R-type
            fout << "\nRd              " << rd[2]; //output RegisterRd
        fout << "\nControl signals ";
        bitset<5>s2(ctrlsig[2]); //5-bit control signals
        fout << s2 << "\n\n";

        //***************MEM/WB***********************//
        fout << "MEM/WB :\n";
        fout << "ReadData        ";
        if(ctrlsig[3]&8) //lw
            fout << ALUout2[3];
        else
            fout << 0;
        fout << "\nALUout          ";
        if(ctrlsig[3]&4) //sw
            fout << 0;
        else
            fout << ALUout1[3];
        fout << "\nControl signals ";
        bitset<2>s3(ctrlsig[3]); //2-bit control signals
        fout << s3 << "\n";
        fout << "=================================================================" << "\n";

        //***************MEM/WB***********************//
        if(ctrlsig[3]&2) //lw , R-type
        {
            if(ctrlsig[3]&1) //lw
                reg[rt[3]] = ALUout2[3];
            else             //R-type
                reg[rd[3]] = ALUout2[3];
            reg[0] = 0; //$0 always equals 0
        }

        //***************EX/MEM***********************//
        if(ctrlsig[2]&4) //sw
            mem_acc[ALUout1[2] / 4] = reg[rt[2]];
        else if(ctrlsig[2]&8) //lw
            ALUout2[2] = mem_acc[ALUout1[2] / 4];
        else
            ALUout2[2] = ALUout1[2];

        //***************EX/MEM->MEM/WB***************//
        ctrlsig[3] = ctrlsig[2];
        ALUout1[3] = ALUout1[2];
        ALUout2[3] = ALUout2[2];
        ins[3] = ins[2];
        name[3] = name[2];
        rs[3] = rs[2];
        rt[3] = rt[2];
        rd[3] = rd[2];
        im[3] = im[2];

        //***************ID/EX************************//
        if(name[1] > 2) //R-type
        {
            if(name[1] == 3) //add
                ALUout1[1] = A + B;
            if(name[1] == 4) //sub
                ALUout1[1] = A - B;
            if(name[1] == 5) //and
                ALUout1[1] = A & B;
            if(name[1] == 6) //or
                ALUout1[1] = A | B;
        }
        else
        {
            if(name[1] == 2) //beq
            {
                if(A == B)
                {
                    PC += im[1] - 1;
                    ctrlsig[0] = 0;
                }
            }
            else //lw , sw
                ALUout1[1] = A + im[1];
        }

        //***************ID/EX->EX/MEM****************//
        ctrlsig[2] = ctrlsig[1];
        ALUout1[2] = ALUout1[1];
        ins[2] = ins[1];
        name[2] = name[1];
        rs[2] = rs[1];
        rt[2] = rt[1];
        rd[2] = rd[1];
        im[2] = im[1];

        //***************IF/ID->ID/EX*****************//
        ctrlsig[1] = ctrlsig[0];
        ALUout1[1] = ALUout1[0];
        ins[1] = ins[0];
        name[1] = name[0];
        rs[1] = rs[0];
        rt[1] = rt[0];
        rd[1] = rd[0];
        im[1] = im[0];

        //*********new instruction->IF/ID************//
        ALUout1[0] = 0;
        PC++;
        ins[0] = op_ins[PC - 1];
        name[0] = op[PC - 1];
        rs[0] = op_rs[PC - 1];
        rt[0] = op_rt[PC - 1];
        im[0] = op_im[PC - 1];
        rd[0] = op_rd[PC - 1];

        //stall//
        stall = false;
        if(ctrlsig[1]&8) //ID/EX.MemRead(lw)
        {
            if(rt[1] == rs[0] || rt[1] == rt[0])
               stall = true;
            //ID/EX.RegisterRt = IF/ID.RegisterRs or ID/EX.RegisterRt = IF/ID.RegisterRt
        }
        if(stall) //Preserve the PC and IF/ID pipeline registers
        {
            ins[0].clear();
            name[0] = 0;
            rs[0] = 0;
            rt[0] = 0;
            rd[0] = im[0] = 0;
            ctrlsig[0] = 0; //Deasserting all nine control signals in the EX, MEM and WB stage
            PC--;
            continue;
        }

        //store 0-stage's control signals//
        if(!ins[0].empty()) //exist instruction
        {
            if(name[0] > 2)       //R-type
                ctrlsig[0] = 386;
            else if(name[0] == 2) //beq
                ctrlsig[0] = 80;
            else if(name[0] == 1) //sw
                ctrlsig[0] = 36;
            else if(name[0] == 0) //lw
                ctrlsig[0] = 43;
            else
                ctrlsig[0] = 0; //empty instruction
        }
        else
            ctrlsig[0] = 0; //empty instruction
    }
    fin.close(); //close file
    return 0;
}
