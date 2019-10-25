# TFTP Server

## Role of Team Members

* Luming Xu
  - RRQ
  - Binary file test cases

* Akhilesh Rawat
  - WRQ
  - Ascii File transfer for WRQ and RRQ
  - Ascii file test cases

## Bonus Points

We implement the WRQ feature in the server. The client can write both binary and ascii files to the server and retrieve them later. The files maintain their format and are identical to the line endings as is desired. We test the following features:
  - transfer binary files of sizes 2047B, 2048B and 3MB from client to the server and retrieve them later
  - transfer ascii files with different line endings from client to the server and retrieve them later
  - timeout if the client disconnects while transfer
  - simultaneous writes to the server.
  - error if the file already exists at the server

