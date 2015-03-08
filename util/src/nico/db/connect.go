package db

import (
	"database/sql"
	"fmt"
	_ "github.com/go-sql-driver/mysql"
)

const DB = "nico"
const Server = "192.168.1.130:3306"

func Connect() *sql.DB {
	var err error
	db, err := sql.Open("mysql", fmt.Sprintf("%s:%s@tcp(%s)/%s", User, Password, Server, DB))
	if err != nil {
		panic(err)
	}
	return db
}
