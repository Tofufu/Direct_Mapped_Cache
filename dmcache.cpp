

//
//  dmcache.cpp
//
//
//  Created by Donna Chow on 12/7/14.
//
//

/*
 
 MAIN ERROR:
 address for a block stored in memory is actually...
 tag|linenumber|00
 
 so...
 0x002D w/ tag = 8 bits, line # = 6 bits, offset = 2 bits...
 0000 0000 0010 1101
 we get rid of offset....and then take
 0000 0000 0010 11 . 00 <-- we add a 00 at the end!
 Thus..... 0000 0000 0010 1100 = 44! We store at 44!
 
 And to retrieve the block... we go in via offset since each block will have [4] w/in main memory
 
 PROBLEM...
 I kept storing the blocks via their 'tag' in their line so a lot of the times it got overwritten!
 For example:
    2104 and 21F6!
 Both have same tags but different lines! One ends up getting overwrriten in main memory!
 */


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
    int addressing;
    
    line():dirty(0), tag(-1), addressing(0)
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
    int addressing; //location we're putting into our main memory
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
    
    //new
    int addressing_twos;
    int addressing_temp = 0;
    int addressing_counter = 2;
    
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
        
        //new
        addressing_twos = pow(2,addressing_counter);
        addressing_temp = addressing_temp + (addressing_twos*binary);
        ++addressing_counter;
        
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
        
        //new
        addressing_twos = pow(2,addressing_counter);
        addressing_temp = addressing_temp + (addressing_twos*binary);
        ++addressing_counter;
        
        address = address / 2;
    }
    temp.tag = actual_dec;
    
    //new
    temp.addressing = addressing_temp;
    
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
    
    //new
    int a = 0;
    
    
    ifstream testing_input(argv[1]);    //input file name = argv[1]
    ofstream outfile("dm-out.txt");
    outfile << hex << setfill('0') << uppercase;
    
    /*  file input line -> hex to decimal -> address
        file input line -> hex to decimal -> read_write = 0/255
        file input line -> hex to decimal -> data
    */
    
    
    while(testing_input >> hex>> address >> read_write >> data)
    {
        
        input_address = dec_binary( address ); //returns tag, linenum, offset
        
        
        a = input_address.addressing ;
       /* if(read_write == 0)
        {
            cout<<"READ! ";
        }
        else
        {
            cout<<"WRITE! ";
        }
        cout<<hex<<input_address.tag<<" "<<input_address.line_number<<" "<<input_address.offset<<"\n";*/
    
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
                    //new
                    (cache[ (input_address.line_number)] ).addressing = input_address.addressing;
                    //retrieve items from block w/in main memory.. aka mainmem[tag]
                    //replace whole lines
                    for(int i = 0; i < 4; i++)
                    {
                        (cache[ (input_address.line_number)]).block_data[i] = (main_memory[ a ]).words[i];
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
                        (main_memory[ (cache[ (input_address.line_number)] ).addressing ]).words[i] = (cache[ (input_address.line_number) ]).block_data[i];
                    }
                    
                    //replacing now with items in cache w/ those from main memory
                    for(int i = 0; i<4; i++)
                    {
                        (cache[ (input_address.line_number)]).block_data[i] = (main_memory[ a ]).words[i];
                    }
                    
                    
                    //change tag not that we've replaced!
                    //setting tag of line to be the new tag (block num)
                    (cache[ (input_address.line_number)] ).tag = input_address.tag;
                    //new
                    (cache[ (input_address.line_number)] ).addressing = input_address.addressing;
                    // no longer dirty! set dirty = 0
                    (cache[ (input_address.line_number)]).dirty = 0;
                    
                    
                    output_data = (cache[ (input_address.line_number)]).block_data[ (input_address.offset)];
                    output_hit = 0; //MISS
                    output_dirty = 0;
                }
            } //MISS
           
            //       THIS IS FOR TESTING/DEBUGGING! PRINTS OUT TO TERMINAL!
           // cout<<"This is the input:\n"<<hex<<" "<<address<<" "<<read_write<<" "<<data<<"\n";
            //cout<<"THIS IS THE OUTPUT:\n"<<hex<<output_data<<" ";
           //cout<<output_hit<<" "<<output_dirty<<"\n----------\n";
            
            //              OUTPUT INTO DM-OUT.TXT      !!!!!
            outfile<<setw(2)<<(output_data & 0xFF)<<' '<<output_hit<<' '<<output_dirty <<endl;
        } //IF READ
        
        
        //WRITE!!!!
        if( read_write == 255)
        {
            // HIT! We have the same block in the line as input for read!
            if( input_address.tag == (cache[ (input_address.line_number) ]).tag)
            {
                (cache[ (input_address.line_number) ]).block_data[ (input_address.offset) ] = data;
                //it's dirty now!
                (cache[ (input_address.line_number) ]).dirty = 1;
            }
            
            //Else we just missed it! so we have to replace it in the cache
            else
            {
                //else if it's dirty we have to write back first before we replace it in the cache
                if( (cache[ (input_address.line_number) ]).dirty == 1)
                {
                    //write back
                    for(int i = 0; i < 4; i++)
                    {
                        (main_memory[ (cache[ (input_address.line_number)] ).addressing ]).words[i] = (cache[ (input_address.line_number) ]).block_data[i];
                    }
                    
                    //replace
                    (cache[ (input_address.line_number) ]).tag = input_address.tag;
                    //new
                    (cache[ (input_address.line_number)] ).addressing = input_address.addressing;
                    
                    for(int i = 0; i < 4; i++)
                    {
                      (cache[ (input_address.line_number)]).block_data[i] = (main_memory[ a ]).words[i];
                    }
                    
                    //WRITING NOW!
                    (cache[ (input_address.line_number) ]).block_data[(input_address.offset)] = data;
                    
                    //ALREADY DIRTY! Don't need to set again!
                    
                    
                } //dirty
                
                //else not dirty so we just replace + write over
                else
                {
                    //replace
                    (cache[ (input_address.line_number) ]).tag = input_address.tag;
                    //new
                    (cache[ (input_address.line_number)] ).addressing = input_address.addressing;
                    for(int i = 0; i < 4; i++)
                    {
                        (cache[ (input_address.line_number)]).block_data[i] = (main_memory[ a ]).words[i];
                    }
                    
                    //WRITING NOW!
                    (cache[ (input_address.line_number) ]).block_data[(input_address.offset)] = data;
                    
                    //It's dirty now!
                    (cache[ (input_address.line_number) ]).dirty = 1;
                }
                
            }//miss
            
        }//read
    }
    
    testing_input.close();
    outfile.close();
    return 0;
}




