package db

import (
	"database/sql"
	"fmt"
	_ "github.com/go-sql-driver/mysql"
)

const DB = "nico"

func Connect(server string) *sql.DB {
	var err error
	db, err := sql.Open("mysql", fmt.Sprintf("%s:%s@tcp(%s:3306)/%s", User, Password, server, DB))
	if err != nil {
		panic(err)
	}
	return db
}
