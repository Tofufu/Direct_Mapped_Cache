//
//  dmcache.cpp
//  
//
//  Created by Donna Chow on 12/7/14.
//
//
#include <iomanip>
#include <math.h>
#include <iostream>
#include <fstream>      
/*
    fstream = stream class to let us read/write from and to files.
    
    Writing to a file:
    ofstream filename;
    filename.open("name.txt");
    filename << "Writing this into file.\n";
    filename.close();
 
    Reading from a file:
    string line;
    ifstream filevar("name.txt");
    getline(filevar, line);
*/



using namespace std; //so we don't have to do std::cout and std::hex

/* 
    These are the individual lines w/in cache 
    2^6 = 64 lines in cache
 
    Dirty Bit | Tag [block #] | Data 
    
    Line number = Index in Cache! ->6 bits
 
    block_data =    data w/in the particular block of main mem.
                    offset received from input = index.
*/



class line
{
    public:
    int dirty;
    int block_data[4];
    int tag;
    
    line():dirty(0), tag(-1)
    {
        for(int i = 0; i < 4; i++)
        {
            block_data[i] = 0;
        }
    }
};


class memory_address
{
    public:
    int tag;
    int line_number;
    int offset;
};

class block
{
public:
    int words[4];
    
    block()
    {
        for(int i = 0; i < 4; i++)
        {
            words[i] = 0;
        }
    }
};



/*  Function declarations   */
memory_address dec_binary(int address);


/*  MEMORY ADDRESS CONVERSION
    Converting a decimal to a 'binary'
    25/2 = newnum, 25%2 = LSB
    LSB ->  MSB
    2 bits for offset
    6 bits for line number
    8 bits for tag (aka block num)

 
 *** C++ IS PASS BY VALUE SO WHEN WE DO address/2... it will not affect actual! ****
 
    This is used for find offset, line number, and block number (tag) from input 16 bit memory address.
*/
memory_address dec_binary(int address)
{
    memory_address temp;
    //initialize all to 0 just incase 'address' reaches 0 already after looping...like tag.
    temp.tag = 0;
    
    
    int binary, twos;
    int actual_dec = 0;
    
    // offset 2 bits
    for(int i = 0; i < 2; i++)
    {
        binary = address % 2;
        twos = pow(2, i);
        actual_dec = actual_dec + (twos * binary); //2^0 (1)...
        address = address / 2;
    }
    
    temp.offset = actual_dec;
    
    actual_dec = 0; //reset
    //line number. 6 bits.
    for(int i = 0; i < 6; i++)
    {
        binary = address % 2;
        twos = pow(2, i);
        actual_dec = actual_dec + (twos * binary);
        address = address / 2;
    }
    
    temp.line_number = actual_dec;
    
    
    actual_dec = 0;
    int counter = 0;
    //tag = 8 bits...counter = 7 here
    while(address != 0)         //if address already = 0 then tag = 0.
    {
        binary = address % 2;
        twos = pow(2, counter);
        actual_dec = actual_dec + (twos * binary);
        ++counter;
        address = address / 2;
    }
    temp.tag = actual_dec;
    
    return temp;
}
    


/*  Reading from Command line:
    argc = argument count
    argv = argument vector
 
    argv[1] = testfile
 
 */


int main(int argc, char** argv)
{
    line cache[64]; //Cache size = 64 lines.
    block main_memory[65536]; //index will be block #(tag) and block has array
    memory_address input_address;
    int address, read_write, data;
    line checking;
    
    
    int output_data, output_hit, output_dirty;
    
    
    
    ifstream testing_input(argv[1]);    //input file name = argv[1]
    
    
    /*  file input line -> hex to decimal -> address
        file input line -> hex to decimal -> read_write = 0/255
        file input line -> hex to decimal -> data
    */
    
    
    while(testing_input >> hex>> address >> read_write >> data)
    {
        input_address = dec_binary( address ); //returns tag, linenum, offset
        
        //  READ!
        if(read_write == 0)
        {
            //go to the line in the cache and store temporarly for checking
            checking = cache[ (input_address.line_number) ];
            
            //HIT!!!
            if( input_address.tag == checking.tag)
            {
                output_data = checking.block_data[ (input_address.offset) ];
                output_hit = 1;
                output_dirty = checking.dirty;
            }
            
            //MISS!!
            if( input_address.tag != checking.tag)
            {
                
                //              NOT DIRTY + MISS
                //if line not dirty we can just replace with stuff from main memory.
                if( checking.dirty == 0)
                {
                    //setting tag of line to be the new tag (block num)
                    (cache[ (input_address.line_number)] ).tag = input_address.tag;
                    
                    //retrieve items from block w/in main memory.. aka mainmem[tag]
                    //replace whole lines
                    for(int i = 0; i < 4; i++)
                    {
                        (cache[ (input_address.line_number)]).block_data[i] = (main_memory[ (input_address.tag) ]).words[i];
                    }
                    
                    //Time to print the specific item now that we're done transferring!
                    
                    output_data = (cache[ (input_address.line_number)]).block_data[ (input_address.offset)];
                    output_hit = 0; //MISS
                    output_dirty = 0;
                    
                }
                
                //          else DIRTY!
                //          NEED TO WRITE BACK BEFORE COPYING FORWARD.
                else
                {
                    //writing back
                    for(int i = 0; i < 4; i++)
                    {
                        (main_memory[ (cache[ (input_address.line_number) ]).tag ]).words[i] = (cache[ (input_address.line_number) ]).block_data[i];
                    }
                    
                    //replacing now with items in cache w/ those from main memory
                    for(int i = 0; i<4; i++)
                    {
                        (cache[ (input_address.line_number)]).block_data[i] = (main_memory[ (input_address.tag) ]).words[i];
                    }
                    
                    // no longer dirty! set dirty = 0
                    (cache[ (input_address.line_number)]).tag = 0;
                    
                    
                    output_data = (cache[ (input_address.line_number)]).block_data[ (input_address.offset)];
                    output_hit = 0; //MISS
                    output_dirty = 0;
                }
            } //MISS
            cout<<output_data<<" "<<output_hit<<" "<<output_dirty<<"\n---------\n";
        } //IF READ
    }
    return 0;
}




