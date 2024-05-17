# CTar
CTar is a custom implementation of the tar archiving utility, written in C. It provides functionalities to create, extract, and list the contents of tar archive files, similar to the standard Unix tar command. This project aims to offer a deeper understanding of file archiving processes and the tar format.

The first option to tar is a mode indicator from the following list:

-c Create a new archive containing the specified items.
-r Like -c, but new entries are appended to the archive. The -f option is required.
-t List archive contents to stdout.
-u Like -r, but new entries are added only if they have a modification date newer than the corresponding entry in the archive. The -f option is required.
-x Extract to disk from the archive. If a file with the same name appears more than once in the archive, each copy will be extracted, with later copies overwriting (replacing) earlier copies.
In -c, -r, or -u mode, each specified file or directory is added to the archive in the order specified on the command line. By default, the contents of each directory are also archived.

## Description
Using getpwuid, getgrgid, lstat for getting information about files.
Create file header.

Using open, write, read to write/read the file header and file content into/from the archive file. 

## Installation
make

## Usage
"  List:    ./my_tar -tf <archive-filename>\n"
"  Extract: ./my_tar -xf <archive-filename>\n"
"  Create:  ./my_tar -cf <archive-filename> [filenames...]"
"  Update:    ./my_tar -uf <archive-filename>\n"
