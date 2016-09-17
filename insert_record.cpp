
#include "sqlite3.h"
#include <iostream>
#include <string>

using namespace std;


//===========================================================================
// C style function to insert student & teacher

bool InsertStudent(sqlite3* db, const string& name, int age)
{
	if (!db) return false;

	sqlite3_stmt* stmt = nullptr;

	if (sqlite3_prepare(db,
		"INSERT INTO students (name, age) VALUES (?,?)",
		-1, &stmt, 0) != SQLITE_OK)
		return false;

	int index = 0;

	if (sqlite3_bind_text(stmt, ++index,
		name.c_str(), name.length(), SQLITE_STATIC) != SQLITE_OK)
		return false;

	if (sqlite3_bind_int(stmt, ++index, age) != SQLITE_OK)
		return false;

	return (sqlite3_step(stmt) != SQLITE_DONE);
}

bool InsertTeacher(sqlite3* db, const string& name,
	const string& subject, int salary)
{
	if (!db) return false;

	sqlite3_stmt* stmt = nullptr;

	if (sqlite3_prepare(db,
		"INSERT INTO teachers (name, subject, salary) VALUES (?,?,?)",
		-1, &stmt, 0) != SQLITE_OK)
		return false;

	int index = 0;

	if (sqlite3_bind_text(stmt, ++index,
		name.c_str(), name.length(), SQLITE_STATIC) != SQLITE_OK)
		return false;

	if (sqlite3_bind_text(stmt, ++index,
		subject.c_str(), subject.length(), SQLITE_STATIC) != SQLITE_OK)
		return false;

	if (sqlite3_bind_int(stmt, ++index, salary) != SQLITE_OK)
		return false;

	return (sqlite3_step(stmt) != SQLITE_DONE);
}


//===========================================================================
// functions to bind something

void bind(sqlite3_stmt* stmt, int index, int i)
{
	if (sqlite3_bind_int(stmt, index, i) != SQLITE_OK)
		throw runtime_error("failed sqlite3_bind_int");
}

void bind(sqlite3_stmt* stmt, int index, const string& str)
{
	if (sqlite3_bind_text(stmt, index, str.c_str(), str.length(), SQLITE_TRANSIENT) != SQLITE_OK)
		throw runtime_error("failed sqlite3_bind_text");
}


//===========================================================================
// variadic template with recursive expansion

template <typename ... Ts> struct Binder;

template <> struct Binder<>
{
	static void Bind(sqlite3_stmt* stmt, int& index) {}
};

template <typename T, typename ... Ts> struct Binder<T, Ts...>
{
	static void Bind(sqlite3_stmt* stmt, int& index, T arg, Ts ... args)
	{
		bind(stmt, ++index, arg);
		Binder<Ts...>::Bind(stmt, index, args...);
	}
};

template <typename ... Ts> void BindAllRecursive(sqlite3_stmt* stmt, Ts ... args)
{
	int index = 0;
	Binder<Ts...>::Bind(stmt, index, args...);
}


//===========================================================================
// variadic template with parameter pack expansion

template <typename ... Ts> void BindAllParamPack(sqlite3_stmt* stmt, const Ts& ... args)
{
	int index = 0;
	(void) initializer_list<int> {(bind(stmt, ++index, args), 0)... };
}


//===========================================================================
// variadic template with fold expression (C++17)

template<typename ... Ts> void BindAllFoldExpr(sqlite3_stmt* stmt, const Ts& ... args)
{
	int index = 0;
	(bind(stmt, ++index, args), ...);
}


//===========================================================================
// generic insert function

template<typename ... Ts> bool Insert(sqlite3* db, const string& q, const Ts& ... args)
{
	if (!db) return false;

	sqlite3_stmt* stmt = nullptr;

	if (sqlite3_prepare(db, q.c_str(), -1, &stmt, 0) != SQLITE_OK)
		return false;

	try
	{
		BindAllRecursive(stmt, args...); // or BindAllRecursive, or BindAllFoldExpr
	}
	catch (const runtime_error& e)
	{
		cout << e.what();
		return false;
	}

	return (sqlite3_step(stmt) != SQLITE_DONE);
}

//===========================================================================
// main function to test the functions

int main()
{
	sqlite3* db = nullptr;
	sqlite3_open("example.db", &db);

	Insert(db, "INSERT INTO students (name, age) VALUES (?,?)",
		"Jane", 16);

	Insert(db, "INSERT INTO teachers (name, subject, salary) VALUES (?,?,?)",
		"Mr. Smith", "Math", 50000);

	if (db) sqlite3_close(db);
}
