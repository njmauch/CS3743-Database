package cs3743;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Date;

public class P3Program 
{
    private Connection connect = null;
    
    private Statement statement = null;
    private PreparedStatement preparedStatement = null;
    private ResultSet resultSet = null;
    public static final int ER_DUP_ENTRY = 1062;
    public static final int ER_DUP_ENTRY_WITH_KEY_NAME = 1586;
    public static final String[] strFlightIdM =
    {   "510"
       ,"705"
       ,"331"
       ,"END"
    };
    
    public P3Program (String user, String password) throws Exception
    {
        try
        {
            // This will load the MySQL driver, each DBMS has its own driver
            Class.forName("com.mysql.jdbc.Driver");
            this.connect = DriverManager.getConnection
                    ("jdbc:mysql://10.100.1.81:3306/nja859db"
                    , user
                    , password);
        }
        catch (Exception e) 
        {
            throw e;
        } 
        
    }
    
    /**
     * Makes a series of database calls performing various commands such
     * as selects, inserts, updates, and deletes.  Will throw exception if 
     * found.  Uses printCustomers to print customer information and uses
     * printUtility function from MySqlUtility to print the results of the 
     * other commands called.
     * 
     * @throws Exception
     */
    public void runProgram() throws Exception 
    {
        try 
        {
            // your code
        	//Part 3 a and b
        	//Select all customers and columns from the Customer table and then 
        	//print the results to the output
        	statement = connect.createStatement();
        	resultSet = statement.executeQuery("select c.* from Customer c");
        	printCustomers("Begging Customers", resultSet);
        	
        	//Part 3 c and d
        	//Select all flights and columns from the Flight table and then
        	//print the results to the output
        	resultSet = statement.executeQuery("select f.* from Flight f");
        	MySqlUtility.printUtility("Beginning Flights", resultSet);
        	
        	//Part 3 e and f
        	//Insert new customer into the customer table, uses a try/catch to catch
        	//any errors, especially if there was a duplicate key error.  Will
        	//then print the customers tables to the screen with the new customer added
        	try {
        	statement.executeUpdate("insert into Customer "
        			+ "(`custNr`, `name`, `preferAirline`, `birthDt`, `gender`)"
        			+ "values(1999, 'Nathan Mauch', 'Alaska Airline', '1988-05-11', 'M')");
        	}
        	catch (SQLException e) {
        		switch (e.getErrorCode()) {
        	        case ER_DUP_ENTRY:
        	        case ER_DUP_ENTRY_WITH_KEY_NAME:
        	            System.out.printf("Duplicate key error: %s\n", e.getMessage());
        	            break;
        	        default:
        	            throw e;
        	    }
        		
        	}
        	catch (Exception e) {
        		throw e;
        	}        	
        	resultSet = statement.executeQuery("select c.* from Customer c");
        	printCustomers("Customers after I was added", resultSet);
        	
        	//Part 3 g and h
        	//Execute query to get the schemas and indexes on the database and then print
        	//them to the output
        	resultSet = statement.executeQuery("select TABLE_SCHEMA, TABLE_NAME, INDEX_NAME\r\n" + 
        			"     , SEQ_IN_INDEX, COLUMN_NAME, CARDINALITY \r\n" + 
        			"  from INFORMATION_SCHEMA.STATISTICS \r\n" + 
        			" where TABLE_SCHEMA = \"nja859db\" \r\n" + 
        			"   and TABLE_NAME = \"Reservation\" \r\n" + 
        			"order by INDEX_NAME, SEQ_IN_INDEX;\r\n");
           	MySqlUtility.printUtility("My Reservation Indexes",  resultSet);
        	
           	//Part 3 h and i
           	//Create a prepared statement to insert new reservations for new customer added.
           	//Will get the flightId's from strFlightIdM array and add each flight reservation.
           	//The reqSeatQty will start at 1 and go up by 1 as it goes to the next flight.
        	preparedStatement = connect.prepareStatement("insert into Reservation values "
        			+ "(?, ?, ?)");   			
            int custNr = 1999;
            int reqSeatQty = 1;
        	for(int i = 0; strFlightIdM[i] != "END"; i++) {
        		preparedStatement.setInt(1,  custNr);
        		preparedStatement.setString(2, strFlightIdM[i]);
        		preparedStatement.setInt(3,  reqSeatQty);
            reqSeatQty += 1;
        		try {
        		    // Execute that insert statement
        		    preparedStatement.executeUpdate();
        		}
        		catch (SQLException e) {
        		    switch (e.getErrorCode())
        		    {
        		        case ER_DUP_ENTRY:
        		        case ER_DUP_ENTRY_WITH_KEY_NAME:
        		            System.out.printf("Duplicate key error: %s\n", e.getMessage());
        		            break;
        		        default:
        		            throw e;
        		    }
        		}
        		catch (Exception e)
        		{
        		    throw e;
        		}
        	}

        	//Part 3 j
        	//Use a prepared statement to select all reservations and columns from reservation table 
        	//where the custNr = 1999 and then print the results to the output
        	preparedStatement = connect.prepareStatement("Select r.* from Reservation r where custNr= ?");
        	preparedStatement.setInt(1, 1999);
        	resultSet = preparedStatement.executeQuery();
        	MySqlUtility.printUtility("My reservations", resultSet);
        	
        	//Part 3 k
        	//Run a query on the reservation table to get all the flightId's, custNr's, and reqSeatQty's from the reservation
        	//table for customers that are on flights with customer 1999 is on but does not include customer 1999 in the result.
        	//Then print those results to the output
        	resultSet = statement.executeQuery("select  r.flightId, r.custNr, r.reqSeatQty from Reservation r, Reservation r1999 " +
        	"where r.custNr <> 1999 and r1999.custNr = 1999 and r.flightId = r1999.flightId");
        	MySqlUtility.printUtility("Other customers on my flgihts", resultSet);
        	
        	//Part 3 l, m, n
        	//Run an update on the reservation table to update the reqSeatQty to be double for any reservation
        	//customer 1999 has, then select all rows and columns from the reservation table and print those results
        	//to the output with the modified reservations
        	statement.executeUpdate("update Reservation SET reqSeatQty = reqSeatQty*2 Where custNr = 1999");
        	resultSet = statement.executeQuery("select r.* from nja859db.Reservation r");
        	MySqlUtility.printUtility("Modified Reservations", resultSet);
        	
        	//Part 3 o
        	//Run a query to get the flightId of any flight that has more than 2 reservations and then print those results to
        	//the output
        	resultSet = statement.executeQuery("select r.flightId, COUNT(*) from Reservation r group by flightId having COUNT(*) > 2");
        	MySqlUtility.printUtility("Flights Having more than 2 reservations", resultSet);
        	
        	//Part 3 p
        	//Delete all reservations from the reservation table for customer 1999
        	statement.executeUpdate("DELETE From Reservation where custNr = 1999");
        	
        	//Part 3 q
        	//Repeat part j
        	//Use a prepared statement to select all reservations and columns from reservation table 
        	//where the custNr = 1999 and then print the results to the output. This will produce no results.
        	preparedStatement = connect.prepareStatement("Select * from Reservation where custNr= ?");
        	preparedStatement.setInt(1, 1999);
        	resultSet = preparedStatement.executeQuery();
        	MySqlUtility.printUtility("My reservations after deleting", resultSet);
        	   	
        	//Part 3 r
        	//Repeat part k
        	//Run a query on the reservation table to get all the flightId's, custNr's, and reqSeatQty's from the reservation
        	//table for customers that are on flights with customer 1999 is on but does not include customer 1999 in the result.
        	//Then print those results to the output.  This will produce no results.
        	resultSet = statement.executeQuery("select  r.flightId, r.custNr, r.reqSeatQty from Reservation r, Reservation r1999 " +
                	"where r.custNr <> 1999 and r1999.custNr = 1999 and r.flightId = r1999.flightId");
            MySqlUtility.printUtility("Other customers on my flgihts", resultSet);
        } 
        catch (Exception e) 
        {
            throw e;
        } 
        finally 
        {
            close();
        }

    }                                                                                                                        
    
    /**
     * Prints out the header for the customers table and then will print 
     * every row after that is in resultSet.
     * @param title
     * @param resultSet
     * @throws SQLException
     */
    private void printCustomers(String title, ResultSet resultSet) throws SQLException 
    {
        // Your output for this must match the format of my sample output exactly. 
        // custNr, name, preferAirline, birthDt, gender
        System.out.printf("%s\n", title);
        // your code
        //indent header
        System.out.printf("    ");
        //Print out Column names with padding for formatting
        System.out.printf("%s ", MySqlUtility.padRight("CustNr", 6, ' '));
        System.out.printf("%s ", MySqlUtility.padRight("Name", 30, ' '));
        System.out.printf("%s ", MySqlUtility.padRight("Preferred Airline", 20, ' '));
        System.out.printf("%s ", MySqlUtility.padRight("Birth Dt", 10, ' '));
        System.out.printf("%s ", MySqlUtility.padRight("Gender", 6, ' '));
        System.out.printf("\n");
        
        //loop through each row in resultSet to be displayed in the output
        while (resultSet.next()) {
        	//indent the row being printed
            System.out.printf("    ");
          
        	String birthDtStr; 		//this may be null
        	
        	//get the customer number and then print value to output
        	int custNr = resultSet.getInt("custNr"); 
        	String custNrStr = Integer.toString(custNr);
        	System.out.printf("%s ", MySqlUtility.padRight(custNrStr, 6, ' '));
        	
        	//Get the customers name and print the name to the output
        	String name = resultSet.getString("name");
            System.out.printf("%s ", MySqlUtility.padRight(name, 30, ' '));
            
            //Get the customers preferred airline, which may be null, and print to 
            //the output.  If null print "---"
        	String preferredAirline = resultSet.getString("preferAirline");
        	if (preferredAirline == null)
        		preferredAirline = "---";
        	
        	//Get the customers birth date, which may be null, and print to the output.
        	//If it is null print "---"
            System.out.printf("%s ", MySqlUtility.padRight(preferredAirline, 20, ' '));
        	Date birthDt = resultSet.getDate("birthDt");
        	if (birthDt == null)
        		birthDtStr = "---";
        	else 
        		birthDtStr = birthDt.toString();
            System.out.printf("%s ", MySqlUtility.padRight(birthDtStr, 10, ' '));
            
            //Get the customers gender, which may be null, and print to the output.
            //If this is null print "---"
        	String gender = resultSet.getString("gender");
        	if (gender == null)
        		gender = "---";
          System.out.printf("%s ", MySqlUtility.padRight(gender, 6, ' '));
        	
        	System.out.printf("\n");
        }
    }
    

    // Close the resultSet, statement, preparedStatement, and connect
    private void close() 
    {
        try 
        {
            if (resultSet != null) 
                resultSet.close();

            if (statement != null) 
                statement.close();
            
            if (preparedStatement != null) 
                preparedStatement.close();

            if (connect != null) 
                connect.close();
        } 
        catch (Exception e) 
        {

        }
    }

}