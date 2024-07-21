#include <iostream>
#include <mysql/mysql.h>
#include <sstream>
#include <unordered_map>

using namespace std;

const char* hostname = "127.0.0.1";
const char* user = "root";
const char* pass = "123";
const char* dbname = "perpustakaan";
unsigned int port = 31235;
const char* unixsocket = NULL;
unsigned long clientflag = 0;

MYSQL* connect_db() {
    MYSQL* conn = mysql_init(0);
    if (conn) {
        conn = mysql_real_connect(conn, hostname, user, pass, dbname, port, unixsocket, clientflag);
        if (conn) {
            cout << "Connected to the database successfully." << endl;
        } else {
            cerr << "Connection failed: " << mysql_error(conn) << endl;
        }
    } else {
        cerr << "mysql_init failed" << endl;
    }
    return conn;
}

// Fungsi untuk login sebagai admin
bool loginAdmin() {
    string username, password;
    cout << "Masukkan username admin: ";
    cin >> username;
    cout << "Masukkan password admin: ";
    cin >> password;

    if (username == "admin" && password == "maribacabuku123") {
        cout << "Login sebagai admin berhasil!" << endl;
        return true;
    } else {
        cout << "Username atau password salah. Coba lagi." << endl;
        return false;
    }
}

// Fungsi untuk login sebagai user
bool loginUser(string& username) {
    string password;
    cout << "Masukkan username user: ";
    cin >> username;
    cout << "Masukkan password user: ";
    cin >> password;

    MYSQL* conn = connect_db();
    if (conn) {
        string query = "SELECT password FROM users WHERE username = '" + username + "'";
        if (mysql_query(conn, query.c_str())) {
            cerr << "SELECT failed: " << mysql_error(conn) << endl;
            mysql_close(conn);
            return false;
        }

        MYSQL_RES* res = mysql_store_result(conn);
        if (res == nullptr) {
            cerr << "mysql_store_result failed: " << mysql_error(conn) << endl;
            mysql_close(conn);
            return false;
        }

        MYSQL_ROW row = mysql_fetch_row(res);
        if (row && row[0] == password) {
            cout << "Login sebagai user berhasil!" << endl;
            mysql_free_result(res);
            mysql_close(conn);
            return true;
        } else {
            cout << "Username atau password salah. Coba lagi." << endl;
            mysql_free_result(res);
            mysql_close(conn);
            return false;
        }
    }
    return false;
}

// Fungsi untuk membuat akun user baru
void createUserAccount() {
    string username, password;
    cout << "Masukkan username user baru: ";
    cin >> username;
    cout << "Masukkan password user baru: ";
    cin >> password;

    MYSQL* conn = connect_db();
    if (conn) {
        string query = "INSERT INTO users (username, password) VALUES ('" + username + "', '" + password + "')";
        if (mysql_query(conn, query.c_str())) {
            cerr << "INSERT failed: " << mysql_error(conn) << endl;
        } else {
            cout << "Akun user baru berhasil dibuat." << endl;
        }
        mysql_close(conn);
    }
}

// Fungsi untuk menambahkan buku baru
void addBook() {
    string title, author;
    cout << "Masukkan judul buku: ";
    cin.ignore();
    getline(cin, title);
    cout << "Masukkan penulis buku: ";
    getline(cin, author);

    MYSQL* conn = connect_db();
    if (conn) {
        string query = "INSERT INTO books (title, author, isAvailable) VALUES ('" + title + "', '" + author + "', 1)";
        if (mysql_query(conn, query.c_str())) {
            cerr << "INSERT failed: " << mysql_error(conn) << endl;
        } else {
            cout << "Buku berhasil ditambahkan." << endl;
        }
        mysql_close(conn);
    }
}

// Fungsi untuk menampilkan daftar buku
void displayBooks() {
    MYSQL* conn = connect_db();
    if (conn) {
        if (mysql_query(conn, "SELECT * FROM books")) {
            cerr << "SELECT failed: " << mysql_error(conn) << endl;
            mysql_close(conn);
            return;
        }

        MYSQL_RES* res = mysql_store_result(conn);
        if (res == nullptr) {
            cerr << "mysql_store_result failed: " << mysql_error(conn) << endl;
            mysql_close(conn);
            return;
        }

        MYSQL_ROW row;
        cout << "\nDaftar Buku Perpustakaan:\n";
        while ((row = mysql_fetch_row(res))) {
            cout << "ID: " << row[0] << ", Title: " << row[1] << ", Author: " << row[2];
            if (stoi(row[3]) == 0) {
                cout << " (Dipinjam)";
            }
            cout << endl;
        }

        mysql_free_result(res);
        mysql_close(conn);
    }
}

// Fungsi untuk mengedit buku
void editBook() {
    displayBooks();
    int id;
    string newTitle, newAuthor;
    cout << "Masukkan ID buku yang ingin diedit: ";
    cin >> id;
    cout << "Masukkan judul baru: ";
    cin.ignore();
    getline(cin, newTitle);
    cout << "Masukkan penulis baru: ";
    getline(cin, newAuthor);

    MYSQL* conn = connect_db();
    if (conn) {
        stringstream query;
        query << "UPDATE books SET title = '" << newTitle << "', author = '" << newAuthor << "' WHERE id = " << id;
        if (mysql_query(conn, query.str().c_str())) {
            cerr << "UPDATE failed: " << mysql_error(conn) << endl;
        } else {
            cout << "Buku berhasil diedit." << endl;
        }
        mysql_close(conn);
    }
}

// Fungsi untuk menghapus buku
void deleteBook() {
    displayBooks();
    int id;
    cout << "Masukkan ID buku yang ingin dihapus: ";
    cin >> id;

    MYSQL* conn = connect_db();
    if (conn) {
        stringstream query;
        query << "DELETE FROM books WHERE id = " << id;
        if (mysql_query(conn, query.str().c_str())) {
            cerr << "DELETE failed: " << mysql_error(conn) << endl;
        } else {
            cout << "Buku berhasil dihapus." << endl;
        }
        mysql_close(conn);
    }
}

// Fungsi untuk memilih dan meminjam buku
void borrowBook(const string& username) {
    displayBooks();
    int id, duration;
    cout << "Masukkan ID buku yang ingin dipinjam: ";
    cin >> id;
    cout << "Pilih durasi peminjaman:\n1. 3 hari\n2. 7 hari\n3. 30 hari\nMasukkan pilihan: ";
    cin >> duration;
    switch (duration) {
        case 1:
            duration = 3;
            break;
        case 2:
            duration = 7;
            break;
        case 3:
            duration = 30;
            break;
        default:
            cout << "Pilihan tidak valid." << endl;
            return;
    }

    MYSQL* conn = connect_db();
    if (conn) {
        stringstream query;
        query << "UPDATE books SET isAvailable = 0 WHERE id = " << id;
        if (mysql_query(conn, query.str().c_str())) {
            cerr << "UPDATE failed: " << mysql_error(conn) << endl;
        } else {
            stringstream borrowQuery;
            borrowQuery << "INSERT INTO borrow (username, book_id, duration) VALUES ('" << username << "', " << id << ", " << duration << ")";
            if (mysql_query(conn, borrowQuery.str().c_str())) {
                cerr << "INSERT borrow failed: " << mysql_error(conn) << endl;
            } else {
                cout << "Harap Jaga Dan Kembalikan Buku Ini Tepat Waktu Jika Hilang Atau Telat Dikembalikan Akan Ada Denda, Terimakasih." << endl;
            }
        }
        mysql_close(conn);
    }
}

// Fungsi untuk mengembalikan buku
void returnBook(const string& username) {
    MYSQL* conn = connect_db();
    if (conn) {
        string query = "SELECT b.id, b.title, b.author FROM books b JOIN borrow br ON b.id = br.book_id WHERE br.username = '" + username + "'";
        if (mysql_query(conn, query.c_str())) {
            cerr << "SELECT failed: " << mysql_error(conn) << endl;
            mysql_close(conn);
            return;
        }

        MYSQL_RES* res = mysql_store_result(conn);
        if (res == nullptr) {
            cerr << "mysql_store_result failed: " << mysql_error(conn) << endl;
            mysql_close(conn);
            return;
        }

        MYSQL_ROW row;
        cout << "\nDaftar Buku Yang Dipinjam:\n";
        while ((row = mysql_fetch_row(res))) {
            cout << "ID: " << row[0] << ", Title: " << row[1] << ", Author: " << row[2] << endl;
        }

        int id;
        cout << "Masukkan ID buku yang ingin dikembalikan: ";
        cin >> id;

        stringstream queryUpdate;
        queryUpdate << "UPDATE books SET isAvailable = 1 WHERE id = " << id;
        if (mysql_query(conn, queryUpdate.str().c_str())) {
            cerr << "UPDATE failed: " << mysql_error(conn) << endl;
        } else {
            stringstream queryDelete;
            queryDelete << "DELETE FROM borrow WHERE book_id = " << id << " AND username = '" << username << "'";
            if (mysql_query(conn, queryDelete.str().c_str())) {
                cerr << "DELETE borrow record failed: " << mysql_error(conn) << endl;
            } else {
                cout << "Buku berhasil dikembalikan." << endl;
            }
        }
        mysql_free_result(res);
        mysql_close(conn);
    }
}

int main() {
    int choice;
    while (true) {
        cout << "\nMenu:\n";
        cout << "1. Login Admin\n";
        cout << "2. Login User\n";
        cout << "3. Buat Akun User Baru\n";
        cout << "4. Keluar\n";
        cout << "Masukkan pilihan: ";
        cin >> choice;

        switch (choice) {
            case 1:
                if (loginAdmin()) {
                    while (true) {
                        int adminChoice;
                        cout << "\nMenu Admin:\n";
                        cout << "1. Tambah Buku\n";
                        cout << "2. Tampilkan Semua Buku\n";
                        cout << "3. Edit Buku\n";
                        cout << "4. Hapus Buku\n";
                        cout << "5. Logout\n";
                        cout << "Masukkan pilihan: ";
                        cin >> adminChoice;

                        switch (adminChoice) {
                            case 1:
                                addBook();
                                break;
                            case 2:
                                displayBooks();
                                break;
                            case 3:
                                editBook();
                                break;
                            case 4:
                                deleteBook();
                                break;
                            case 5:
                                goto mainMenu;
                            default:
                                cout << "Pilihan tidak valid. Coba lagi.\n";
                                break;
                        }
                    }
                }
                break;
            case 2: {
                string username;
                if (loginUser(username)) {
                    while (true) {
                        int userChoice;
                        cout << "\nMenu User:\n";
                        cout << "1. Lihat Buku\n";
                        cout << "2. Pilih Buku\n";
                        cout << "3. Kembalikan Buku\n";
                        cout << "4. Logout\n";
                        cout << "Masukkan pilihan: ";
                        cin >> userChoice;

                        switch (userChoice) {
                            case 1:
                                displayBooks();
                                break;
                            case 2:
                                borrowBook(username);
                                break;
                            case 3:
                                returnBook(username);
                                break;
                            case 4:
                                goto mainMenu;
                            default:
                                cout << "Pilihan tidak valid. Coba lagi.\n";
                                break;
                        }
                    }
                }
                break;
            }
            case 3:
                createUserAccount();
                break;
            case 4:
                cout << "Keluar dari program.\n";
                return 0;
            default:
                cout << "Pilihan tidak valid. Coba lagi.\n";
                break;
        }
    mainMenu:
        continue;
    }

    return 0;
}
