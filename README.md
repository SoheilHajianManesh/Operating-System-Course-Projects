# Operating-System-Projects
- [Operating System Projects:](#projects)
    - *1.Socket Programming*
    - *2.MapReduce*
## Projects

### Socket Programming {#socket-programming}

A buyer and a seller are implemented. The seller can add an advertisement by broadcasting it, the buyer can connect to the seller using TCP protocol with the seller's port number which is broadcasted by the seller. The buyer can then send an offer to the seller. If the offer is accepted, the SOLD message is broadcasted by the seller and the buyer can disconnect from the seller. Otherwise, the AVAILABLE message is broadcasted by the seller and the buyer can send another offer.

### MapReduce {#mapreduce}

A MapReduce framework is implemented in order to count the number of books in each genre. The genres are stored in a file called genres.csv and the input files are stored in part#.csv files.
