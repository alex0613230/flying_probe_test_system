// gcc  position_data.c -o position_data -I/usr/include/mysql -lmysqlclient
#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <string.h>

// Parameters used in mysql_real_connect()
static char *host_name = NULL; 		// server host (default=localhost)
static char *user_name = "U0622001"; 	// username (default=login name)
static char *password = "********"; 	// password (default=none)
static unsigned int port_num = 0; 	// port number (use built-in value)
static char *socket_name = NULL; 	// socket name (use built-in value)
static char *db_name = NULL; 		// database name (default=none)
static unsigned int flags = 0; 		// connection flags (none)

static MYSQL *conn;			// pointer to connection handler

int main(int argc, char **argv)
{
	if ((conn = mysql_init(NULL)) == NULL) 
	{
		printf("%s\n", mysql_error(conn));
		exit(1);
	}

	if (mysql_real_connect(conn, host_name, user_name, password,
			db_name, port_num, socket_name, flags) == NULL) 
	{
		printf("%s\n", mysql_error(conn));
		mysql_close(conn);
		exit(1);
	} 
	
	char	sqlstring[300];
	
	if (mysql_select_db(conn, "FPT")) 
		printf("%s\n", mysql_error(conn));
	else	printf("Database FPT select success.\n");

	strcpy(sqlstring, "CREATE TABLE position_data (\
		X_Cur int,\
		Y_Cur int,\
		Z_Cur int)");
	
	if (mysql_query(conn, sqlstring))
		printf("%s\n", mysql_error(conn));
	else	printf("Table create success.\n");
  
	mysql_close(conn);
}

