256 bye Direct-mapped cache with a line size of 4 bytes.
Supports read operation (reading a bye from the cache) and a write operation (writing a new byte of data into the cache).

Write Policy: Write-Back
Write Miss Policy: Write-Allocate

Test file format per line:
16 bit address | Read/Write Operation | 8 bits of Data

Output file format per line:
Byte of data from read operation | HIT/MISS | Dirty Bit
