#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include <time.h>
#include <windows.h>

#define ADMIN_FILE "admins.txt"
#define STUDENT_FILE "students.txt"
#define BOOK_FILE "books.txt"
#define MAX_BOOKS 1000
#define MAX_ISSUED_BOOKS 5
#define MIN_PASSWORD_LEN 8
#define MAX_PASSWORD_LEN 32
#define ISSUED_BOOK_FILE "issued_books.txt"
#define ADMIN_KEY "admin#3x"
#define STUDENT_KEY "student$y1"
#define FINE_PER_DAY 10 // 10 Taka per day for overdue books

const char *weakPasswords[] = {
    "password123", "12345678", "qwerty", "abcdef",
    "password", "iloveyou", "admin", "welcome"
};

struct Book {
    char isbn[20];
    char title[100];
    char author[100];
    char genre[50];
    int totalCopies;
    int availableCopies;
};

struct User {
    char username[50];
    char password[50];
    char email[100];
    char phone[20];
    int issuedBooks[MAX_ISSUED_BOOKS];
    int numIssuedBooks;
};

struct IssuedBook {
    char username[50];
    char isbn[20];
    time_t issueDate;
    time_t dueDate;
    int returned; // 0 = Not returned, 1 = Returned
};

time_t getCurrentTime() {
    return time(NULL);
}

int isUsernameTaken(const char *username, const char *filename) {
    struct User user;
    FILE *file = fopen(filename, "r");
    if (file == NULL) return 0;

    while (fscanf(file, "%s %s %s %s", user.username, user.password, user.email, user.phone) != EOF) {
        if (strcmp(username, user.username) == 0) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

int isEmailTaken(const char *email, const char *filename) {
    struct User user;
    FILE *file = fopen(filename, "r");
    if (file == NULL) return 0;

    while (fscanf(file, "%s %s %s %s", user.username, user.password, user.email, user.phone) != EOF) {
        if (strcmp(email, user.email) == 0) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

int isPhoneTaken(const char *phone, const char *filename) {
    struct User user;
    FILE *file = fopen(filename, "r");
    if (file == NULL) return 0;

    while (fscanf(file, "%s %s %s %s", user.username, user.password, user.email, user.phone) != EOF) {
        if (strcmp(phone, user.phone) == 0) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

int isValidUsername(const char *username) {
    for (int i = 0; i < strlen(username); i++) {
        if (!(isalpha(username[i]) || username[i] == ' ')) {
            return 0;
        }
    }
    return 1;
}

int isValidEmail(const char *email) {
    const char *at = strchr(email, '@');
    const char *dot = strrchr(email, '.');

    return (at && dot && at < dot);
}

int isValidPhone(const char *phone) {
    if (strlen(phone) != 11) return 0;
    if (phone[0] != '0' || phone[1] != '1') return 0;

    for (int i = 0; i < 11; i++) {
        if (!isdigit(phone[i])) return 0;
    }

    return 1;
}

void getMaskedPassword(char *password) {
    char ch;
    int index = 0;

    while (1) {
        ch = getch();

        if (ch == '\r' || ch == '\n') {
            break;
        }

        if (ch == 127 || ch == '\b') {
            if (index > 0) {
                printf("\b \b");
                index--;
            }
        } else if (index < MAX_PASSWORD_LEN - 1) {
            password[index++] = ch;
            printf("*");
        }
    }

    password[index] = '\0';
    printf("\n");
}

int isStrongPassword(const char *password, const char *username) {
    int hasUpper = 0, hasLower = 0, hasDigit = 0, hasSpecial = 0;
    int len = strlen(password);

    if (len < MIN_PASSWORD_LEN || len > MAX_PASSWORD_LEN) {
        printf("Password must be between %d and %d characters long.\n", MIN_PASSWORD_LEN, MAX_PASSWORD_LEN);
        return 0;
    }

    for (int i = 0; i < len; i++) {
        if (isupper(password[i])) hasUpper = 1;
        if (islower(password[i])) hasLower = 1;
        if (isdigit(password[i])) hasDigit = 1;
        if (strchr("!@#$%^&*()-_=+[]{}|;:'\",.<>?/", password[i])) hasSpecial = 1;
    }

    if (!hasUpper || !hasLower || !hasDigit || !hasSpecial) {
        printf("Password must contain at least one uppercase letter, one lowercase letter, one digit, and one special character.\n");
        return 0;
    }

    if (strstr(password, username)) {
        printf("Password should not contain your username.\n");
        return 0;
    }

    for (int i = 0; i < sizeof(weakPasswords) / sizeof(weakPasswords[0]); i++) {
        if (strcmp(password, weakPasswords[i]) == 0) {
            printf("Your password is too weak. Choose a stronger one.\n");
            return 0;
        }
    }

    return 1;
}

void registerUser(const char *role) {
    struct User user;
    char key[50];
    const char *filename = (strcmp(role, "admin") == 0) ? ADMIN_FILE : STUDENT_FILE;
    const char *correctKey = (strcmp(role, "admin") == 0) ? ADMIN_KEY : STUDENT_KEY;
    FILE *file = fopen(filename, "a");

    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }

    printf("Enter the registration key for %s: ", role);
    scanf("%s", key);

    if (strcmp(key, correctKey) != 0) {
        printf("Incorrect registration key! Registration failed.\n");
        fclose(file);
        return;
    }

    do {
        printf("Enter username: ");
        scanf(" %[^\n]%*c", user.username);

        if (!isValidUsername(user.username)) {
            printf("Invalid username! Only letters and spaces are allowed.\n");
        } else if (isUsernameTaken(user.username, filename)) {
            printf("This username is already taken. Please choose another one.\n");
        }
    } while (!isValidUsername(user.username) || isUsernameTaken(user.username, filename));

    do {
        printf("Enter email: ");
        scanf("%s", user.email);

        if (!isValidEmail(user.email)) {
            printf("Invalid email format. Please enter a valid email.\n");
        } else if (isEmailTaken(user.email, filename)) {
            printf("This email is already registered. Try another one.\n");
        } else {
            break;
        }
    } while (1);

    do {
        printf("Enter phone number: ");
        scanf("%s", user.phone);

        if (!isValidPhone(user.phone)) {
            printf("Invalid phone number! It must be exactly 11 digits starting with 01.\n");
        } else if (isPhoneTaken(user.phone, filename)) {
            printf("This phone number is already registered. Try another one.\n");
        } else {
            break;
        }
    } while (1);

    do {
        printf("Enter password: ");
        getMaskedPassword(user.password);
    } while (!isStrongPassword(user.password, user.username));

    fprintf(file, "%s %s %s %s\n", user.username, user.password, user.email, user.phone);
    fclose(file);

    printf("Registration successful as %s!\n", role);
}

char* loginUser(const char *role) {
    static struct User user; // static to preserve after return
    char username[50], password[50];
    const char *filename = (strcmp(role, "admin") == 0) ? ADMIN_FILE : STUDENT_FILE;
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        printf("No %ss registered yet.\n", role);
        return NULL;
    }

    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    getMaskedPassword(password);

    while (fscanf(file, "%s %s %s %s", user.username, user.password, user.email, user.phone) != EOF) {
        if (strcmp(username, user.username) == 0 && strcmp(password, user.password) == 0) {
            printf("Login successful as %s!\n", role);
            fclose(file);
            return user.username;
        }
    }

    printf("Invalid credentials for %s!\n", role);
    fclose(file);
    return NULL;
}

void viewBooks() {
    FILE *file = fopen(BOOK_FILE, "r");
    struct Book book;

    if (file == NULL) {
        printf("No books available.\n");
        return;
    }

    printf("Books available in the library:\n");
    while (fscanf(file, "%s %s %s %s %d %d", book.isbn, book.title, book.author, book.genre, &book.totalCopies, &book.availableCopies) != EOF) {
        printf("ISBN: %s, Title: %s, Author: %s, Genre: %s, Copies: %d, Available: %d\n",
               book.isbn, book.title, book.author, book.genre, book.totalCopies, book.availableCopies);
    }
    fclose(file);
}

void addBook() {
    struct Book book;
    FILE *file = fopen(BOOK_FILE, "a");

    if (file == NULL) {
        printf("Error opening book file.\n");
        return;
    }

    printf("Enter book ISBN: ");
    scanf("%s", book.isbn);
    printf("Enter book title: ");
    scanf(" %[^\n]%*c", book.title);
    printf("Enter book author: ");
    scanf(" %[^\n]%*c", book.author);
    printf("Enter book genre: ");
    scanf(" %[^\n]%*c", book.genre);
    printf("Enter total copies: ");
    scanf("%d", &book.totalCopies);

    book.availableCopies = book.totalCopies;

    fprintf(file, "%s %s %s %s %d %d\n", book.isbn, book.title, book.author, book.genre, book.totalCopies, book.availableCopies);
    fclose(file);
    printf("Book added successfully.\n");
}

void deleteBook() {
    char isbn[20];
    FILE *file = fopen(BOOK_FILE, "r");
    FILE *tempFile = fopen("temp.txt", "w");
    struct Book book;

    if (file == NULL || tempFile == NULL) {
        printf("Error opening files.\n");
        return;
    }

    printf("Enter ISBN of the book to delete: ");
    scanf("%s", isbn);

    int found = 0;
    while (fscanf(file, "%s %s %s %s %d %d", book.isbn, book.title, book.author, book.genre, &book.totalCopies, &book.availableCopies) != EOF) {
        if (strcmp(book.isbn, isbn) != 0) {
            fprintf(tempFile, "%s %s %s %s %d %d\n", book.isbn, book.title, book.author, book.genre, book.totalCopies, book.availableCopies);
        } else {
            found = 1;
        }
    }

    fclose(file);
    fclose(tempFile);

    if (found) {
        remove(BOOK_FILE);
        rename("temp.txt", BOOK_FILE);
        printf("Book deleted successfully.\n");
    } else {
        printf("Book with ISBN %s not found.\n", isbn);
        remove("temp.txt");
    }
}

void issueBook() {
    struct Book book;
    struct IssuedBook issued;
    char username[50];
    char isbn[20];
    int found = 0;

    printf("Enter student username: ");
    scanf("%s", username);
    printf("Enter book ISBN to issue: ");
    scanf("%s", isbn);

    FILE *bookFile = fopen(BOOK_FILE, "r");
    FILE *tempFile = fopen("temp_books.txt", "w");
    FILE *issuedFile = fopen(ISSUED_BOOK_FILE, "a");

    if (!bookFile || !tempFile || !issuedFile) {
        printf("Error opening file(s).\n");
        return;
    }

    while (fscanf(bookFile, "%s %s %s %s %d %d", book.isbn, book.title, book.author,
                  book.genre, &book.totalCopies, &book.availableCopies) != EOF) {
        if (strcmp(book.isbn, isbn) == 0) {
            if (book.availableCopies > 0) {
                found = 1;
                book.availableCopies--;

                time_t now = time(NULL);
                issued.issueDate = now;
                issued.dueDate = now + (7 * 24 * 60 * 60); // 7 days
                strcpy(issued.username, username);
                strcpy(issued.isbn, isbn);
                issued.returned = 0;

                fprintf(issuedFile, "%s %s %ld %ld %d\n", issued.username, issued.isbn,
                        issued.issueDate, issued.dueDate, issued.returned);
                printf("Book issued successfully to %s.\n", username);
            } else {
                printf("No available copies of this book.\n");
            }
        }
        fprintf(tempFile, "%s %s %s %s %d %d\n", book.isbn, book.title, book.author,
                book.genre, book.totalCopies, book.availableCopies);
    }

    if (!found) {
        printf("Book with ISBN %s not found.\n", isbn);
    }

    fclose(bookFile);
    fclose(tempFile);
    fclose(issuedFile);

    remove(BOOK_FILE);
    rename("temp_books.txt", BOOK_FILE);
}

void viewAllIssuedBooks() {
    struct IssuedBook issued;
    FILE *file = fopen(ISSUED_BOOK_FILE, "r");

    if (file == NULL) {
        printf("No books have been issued yet.\n");
        return;
    }

    printf("List of all issued books:\n");
    printf("Username\tISBN\t\tIssue Date\t\tDue Date\tStatus\n");
    printf("----------------------------------------------------------------\n");

    while (fscanf(file, "%s %s %ld %ld %d", issued.username, issued.isbn, &issued.issueDate, &issued.dueDate, &issued.returned) != EOF) {
        char issueDateStr[50], dueDateStr[50];
        struct tm *issueTm = localtime(&issued.issueDate);
        struct tm *dueTm = localtime(&issued.dueDate);

        strftime(issueDateStr, sizeof(issueDateStr), "%Y-%m-%d %H:%M:%S", issueTm);
        strftime(dueDateStr, sizeof(dueDateStr), "%Y-%m-%d %H:%M:%S", dueTm);

        printf("%s\t%s\t%s\t%s\t%s\n", issued.username, issued.isbn, issueDateStr, dueDateStr,
               issued.returned ? "Returned" : "Not Returned");
    }

    fclose(file);
}

void viewIssuedBooksByStudent(const char *username) {
    struct IssuedBook issued;
    FILE *file = fopen(ISSUED_BOOK_FILE, "r");

    if (file == NULL) {
        printf("No books have been issued yet.\n");
        return;
    }

    printf("\n=== Your Issued Books ===\n");
    printf("ISBN\t\tTitle\t\t\tDue Date\t\tStatus\n");
    printf("----------------------------------------------------------------\n");

    int found = 0;
    while (fscanf(file, "%s %s %ld %ld %d",
           issued.username, issued.isbn, &issued.issueDate,
           &issued.dueDate, &issued.returned) != EOF) {
        if (strcmp(issued.username, username) == 0) {
            found = 1;
            // Get book title
            struct Book book;
            FILE *bookFile = fopen(BOOK_FILE, "r");
            char title[100] = "Unknown";
            if (bookFile) {
                while (fscanf(bookFile, "%s %s %*s %*s %*d %*d",
                       book.isbn, book.title) != EOF) {
                    if (strcmp(book.isbn, issued.isbn) == 0) {
                        strncpy(title, book.title, sizeof(title)-1);
                        break;
                    }
                }
                fclose(bookFile);
            }

            // Format due date
            char dueDateStr[50];
            struct tm *dueTm = localtime(&issued.dueDate);
            strftime(dueDateStr, sizeof(dueDateStr), "%Y-%m-%d %H:%M", dueTm);

            printf("%s\t%-20.20s\t%s\t%s\n",
                   issued.isbn, title, dueDateStr,
                   issued.returned ? "Returned" : "Not Returned");
        }
    }

    if (!found) {
        printf("No books issued to you.\n");
    }
    fclose(file);
}


void searchBooks() {
    char query[100];
    int choice;
    FILE *file = fopen(BOOK_FILE, "r");
    struct Book book;
    int found = 0;

    if (file == NULL) {
        printf("No books available.\n");
        return;
    }

    printf("\nSearch Books By:\n");
    printf("1. ISBN\n");
    printf("2. Title\n");
    printf("3. Author\n");
    printf("4. Genre\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);

    printf("Enter search term: ");
    scanf(" %[^\n]%*c", query);

    printf("\nSearch Results:\n");
    printf("------------------------------------------------------------\n");

    while (fscanf(file, "%s %s %s %s %d %d", book.isbn, book.title, book.author, book.genre, &book.totalCopies, &book.availableCopies) != EOF) {
        int match = 0;
        switch (choice) {
            case 1: match = (strstr(book.isbn, query) != NULL); break;
            case 2: match = (strstr(book.title, query) != NULL); break;
            case 3: match = (strstr(book.author, query) != NULL); break;
            case 4: match = (strstr(book.genre, query) != NULL); break;
            default: printf("Invalid choice.\n"); fclose(file); return;
        }

        if (match) {
            found = 1;
            printf("ISBN: %s\nTitle: %s\nAuthor: %s\nGenre: %s\nCopies: %d\nAvailable: %d\n",
                   book.isbn, book.title, book.author, book.genre, book.totalCopies, book.availableCopies);
            printf("------------------------------------------------------------\n");
        }
    }

    fclose(file);

    if (!found) {
        printf("No matching books found.\n");
    }
}

void checkAvailability() {
    char isbn[20];
    FILE *file = fopen(BOOK_FILE, "r");
    struct Book book;

    if (file == NULL) {
        printf("No books available.\n");
        return;
    }

    printf("Enter ISBN to check availability: ");
    scanf("%s", isbn);

    int found = 0;
    while (fscanf(file, "%s %s %s %s %d %d", book.isbn, book.title, book.author, book.genre, &book.totalCopies, &book.availableCopies) != EOF) {
        if (strcmp(book.isbn, isbn) == 0) {
            found = 1;
            printf("\nBook Availability:\n");
            printf("Title: %s\nAuthor: %s\nAvailable Copies: %d/%d\n",
                   book.title, book.author, book.availableCopies, book.totalCopies);
            break;
        }
    }

    if (!found) printf("Book with ISBN %s not found.\n", isbn);
    fclose(file);
}

void calculateFine() {
    char username[50], isbn[20];
    FILE *file = fopen(ISSUED_BOOK_FILE, "r");
    struct IssuedBook issued;
    time_t now = time(NULL);
    int fine = 0;

    if (file == NULL) {
        printf("No books have been issued yet.\n");
        return;
    }

    printf("Enter student username: ");
    scanf("%s", username);
    printf("Enter book ISBN: ");
    scanf("%s", isbn);

    int found = 0;
    while (fscanf(file, "%s %s %ld %ld %d", issued.username, issued.isbn, &issued.issueDate, &issued.dueDate, &issued.returned) != EOF) {
        if (strcmp(issued.username, username) == 0 && strcmp(issued.isbn, isbn) == 0 && !issued.returned) {
            found = 1;
            if (now > issued.dueDate) {
                fine = (now - issued.dueDate) / (60 * 60 * 24) * FINE_PER_DAY;
                printf("\nFine for overdue book: Taka %d\n", fine);
            } else {
                printf("No fine. Book is not overdue.\n");
            }
            break;
        }
    }

    if (!found) printf("No such issued book found.\n");
    fclose(file);
}

void returnBook() {
    char username[50], isbn[20];
    FILE *bookFile = fopen(BOOK_FILE, "r");
    FILE *tempBookFile = fopen("temp_books.txt", "w");
    FILE *issuedFile = fopen(ISSUED_BOOK_FILE, "r");
    FILE *tempIssuedFile = fopen("temp_issued.txt", "w");
    struct Book book;
    struct IssuedBook issued;
    int found = 0;

    if (!bookFile || !tempBookFile || !issuedFile || !tempIssuedFile) {
        printf("Error opening files.\n");
        return;
    }

    printf("Enter student username: ");
    scanf("%s", username);
    printf("Enter book ISBN to return: ");
    scanf("%s", isbn);

    // Update book availability
    while (fscanf(bookFile, "%s %s %s %s %d %d", book.isbn, book.title,
                  book.author, book.genre, &book.totalCopies, &book.availableCopies) != EOF) {
        if (strcmp(book.isbn, isbn) == 0) {
            book.availableCopies++;
            found = 1;
        }
        fprintf(tempBookFile, "%s %s %s %s %d %d\n", book.isbn, book.title,
                book.author, book.genre, book.totalCopies, book.availableCopies);
    }

    // Mark book as returned
    while (fscanf(issuedFile, "%s %s %ld %ld %d", issued.username, issued.isbn,
                  &issued.issueDate, &issued.dueDate, &issued.returned) != EOF) {
        if (strcmp(issued.username, username) == 0 &&
            strcmp(issued.isbn, isbn) == 0 && !issued.returned) {
            issued.returned = 1;
            found = 1;
        }
        fprintf(tempIssuedFile, "%s %s %ld %ld %d\n", issued.username, issued.isbn,
                issued.issueDate, issued.dueDate, issued.returned);
    }

    fclose(bookFile);
    fclose(tempBookFile);
    fclose(issuedFile);
    fclose(tempIssuedFile);

    if (found) {
        remove(BOOK_FILE);
        rename("temp_books.txt", BOOK_FILE);
        remove(ISSUED_BOOK_FILE);
        rename("temp_issued.txt", ISSUED_BOOK_FILE);

        // Reservation notification
        FILE *reserveFile = fopen("reservations.txt", "r");
        FILE *tempReserve = fopen("temp_reservations.txt", "w");
        FILE *notifyFile = fopen("notifications.txt", "a");
        char line[200], resUser[50], resISBN[20];
        time_t startTime, expiry;

        if (reserveFile && tempReserve && notifyFile) {
            while (fgets(line, sizeof(line), reserveFile)) {
                sscanf(line, "%[^|]|%[^|]|%ld|%ld", resUser, resISBN, &startTime, &expiry);
                if (strcmp(isbn, resISBN) == 0 && time(NULL) <= expiry) {
                    fprintf(notifyFile,
                        "User: %s - Your reserved book (ISBN: %s) is now available!\n",
                        resUser, resISBN);
                    // Don't copy this reservation (it’s now fulfilled)
                } else {
                    fputs(line, tempReserve);
                }
            }
            fclose(reserveFile);
            fclose(tempReserve);
            fclose(notifyFile);

            remove("reservations.txt");
            rename("temp_reservations.txt", "reservations.txt");
        }

        printf("Book returned successfully.\n");
    } else {
        printf("No such issued book found.\n");
        remove("temp_books.txt");
        remove("temp_issued.txt");
    }
}

// Function prototypes
void searchBooksMenu();
void viewBookDetails();
void requestBookIssue(const char* username);
void viewIssuedBooks(const char* username);
void requestBookRenewalReturn(const char* username);
void viewBorrowingHistory(const char* username);
void viewNotifications(const char* username);
void manageProfile(const char* username);
void changePassword(const char* username, const char* role);

// Implementations

void searchBooksMenu() {
    printf("\n=== Search Books ===\n");
    searchBooks(); // Reusing the existing function
}

void viewBookDetails() {
    char isbn[20];
    FILE *file = fopen(BOOK_FILE, "r");
    struct Book book;
    int found = 0;

    if (file == NULL) {
        printf("No books available.\n");
        return;
    }

    printf("\nEnter ISBN to view details: ");
    scanf("%s", isbn);

    while (fscanf(file, "%s %s %s %s %d %d", book.isbn, book.title, book.author,
           book.genre, &book.totalCopies, &book.availableCopies) != EOF) {
        if (strcmp(book.isbn, isbn) == 0) {
            found = 1;
            printf("\n=== Book Details ===\n");
            printf("ISBN: %s\n", book.isbn);
            printf("Title: %s\n", book.title);
            printf("Author: %s\n", book.author);
            printf("Genre: %s\n", book.genre);
            printf("Total Copies: %d\n", book.totalCopies);
            printf("Available Copies: %d\n", book.availableCopies);
            printf("====================\n");
            break;
        }
    }

    if (!found) {
        printf("Book with ISBN %s not found.\n", isbn);
    }
    fclose(file);
}

void requestBookIssue(const char* username) {
    printf("\n=== Request Book Issue ===\n");
    issueBook(); // Reusing existing function but will modify to use the logged-in username
}

void viewIssuedBooks(const char* username) {
    printf("\n=== Your Issued Books ===\n");
    viewIssuedBooksByStudent(username);
}

void requestBookRenewalReturn(const char* username) {
    int choice;
    printf("\n=== Book Renewal/Return ===\n");
    printf("1. Renew Book\n");
    printf("2. Return Book\n");
    printf("3. Back to Menu\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);

    switch(choice) {
        case 1: {
            // Book renewal implementation
            char isbn[20];
            printf("Enter ISBN of book to renew: ");
            scanf("%s", isbn);

            FILE *issuedFile = fopen(ISSUED_BOOK_FILE, "r");
            FILE *tempFile = fopen("temp_issued.txt", "w");
            struct IssuedBook issued;
            int found = 0;
            time_t now = time(NULL);

            if (!issuedFile || !tempFile) {
                printf("Error opening files.\n");
                return;
            }

            while (fscanf(issuedFile, "%s %s %ld %ld %d",
                   issued.username, issued.isbn, &issued.issueDate,
                   &issued.dueDate, &issued.returned) != EOF) {
                if (strcmp(issued.username, username) == 0 &&
                    strcmp(issued.isbn, isbn) == 0 &&
                    !issued.returned) {
                    found = 1;
                    // Extend due date by 7 more days
                    issued.dueDate += 7 * 24 * 60 * 60;
                    printf("Book renewed successfully. New due date: %s",
                           ctime(&issued.dueDate));
                }
                fprintf(tempFile, "%s %s %ld %ld %d\n",
                       issued.username, issued.isbn, issued.issueDate,
                       issued.dueDate, issued.returned);
            }

            fclose(issuedFile);
            fclose(tempFile);

            if (found) {
                remove(ISSUED_BOOK_FILE);
                rename("temp_issued.txt", ISSUED_BOOK_FILE);
            } else {
                printf("No such issued book found or already returned.\n");
                remove("temp_issued.txt");
            }
            break;
        }
        case 2:
            returnBook(); // Existing return book function
            break;
        case 3:
            return;
        default:
            printf("Invalid choice!\n");
    }
}

void viewBorrowingHistory(const char* username) {
    struct IssuedBook issued;
    FILE *file = fopen(ISSUED_BOOK_FILE, "r");

    if (file == NULL) {
        printf("No borrowing history available.\n");
        return;
    }

    printf("\n=== Your Borrowing History ===\n");
    printf("ISBN\t\tIssue Date\t\tDue Date\tStatus\n");
    printf("------------------------------------------------------\n");

    while (fscanf(file, "%s %s %ld %ld %d", issued.username, issued.isbn,
           &issued.issueDate, &issued.dueDate, &issued.returned) != EOF) {
        if (strcmp(issued.username, username) == 0) {
            char issueDateStr[50], dueDateStr[50];
            struct tm *issueTm = localtime(&issued.issueDate);
            struct tm *dueTm = localtime(&issued.dueDate);

            strftime(issueDateStr, sizeof(issueDateStr), "%Y-%m-%d %H:%M:%S", issueTm);
            strftime(dueDateStr, sizeof(dueDateStr), "%Y-%m-%d %H:%M:%S", dueTm);

            printf("%s\t%s\t%s\t%s\n", issued.isbn, issueDateStr, dueDateStr,
                   issued.returned ? "Returned" : "Not Returned");
        }
    }
    fclose(file);
}


void manageProfile(const char* username) {
    struct User user;
    FILE *file = fopen(STUDENT_FILE, "r");
    FILE *tempFile = fopen("temp.txt", "w");
    int found = 0;

    if (file == NULL || tempFile == NULL) {
        printf("Error accessing profile data.\n");
        return;
    }

    // Find the user's profile
    while (fscanf(file, "%s %s %s %s", user.username, user.password,
           user.email, user.phone) != EOF) {
        if (strcmp(user.username, username) == 0) {
            found = 1;
            printf("\n=== Your Profile ===\n");
            printf("Username: %s\n", user.username);
            printf("Email: %s\n", user.email);
            printf("Phone: %s\n", user.phone);

            int choice;
            printf("\n1. Update Email\n");
            printf("2. Update Phone\n");
            printf("3. Back to Menu\n");
            printf("Enter choice: ");
            scanf("%d", &choice);

            if (choice == 1) {
                do {
                    printf("Enter new email: ");
                    scanf("%s", user.email);
                    if (!isValidEmail(user.email)) {
                        printf("Invalid email format!\n");
                    } else if (isEmailTaken(user.email, STUDENT_FILE)) {
                        printf("Email already in use!\n");
                    } else {
                        break;
                    }
                } while (1);
            } else if (choice == 2) {
                do {
                    printf("Enter new phone: ");
                    scanf("%s", user.phone);
                    if (!isValidPhone(user.phone)) {
                        printf("Invalid phone number!\n");
                    } else if (isPhoneTaken(user.phone, STUDENT_FILE)) {
                        printf("Phone number already in use!\n");
                    } else {
                        break;
                    }
                } while (1);
            }
        }
        fprintf(tempFile, "%s %s %s %s\n", user.username, user.password,
                user.email, user.phone);
    }

    fclose(file);
    fclose(tempFile);

    if (found) {
        remove(STUDENT_FILE);
        rename("temp.txt", STUDENT_FILE);
        printf("Profile updated successfully!\n");
    } else {
        remove("temp.txt");
        printf("Profile not found!\n");
    }
}

void changePassword(const char* username, const char* role) {
    struct User user;
    char currentPass[50], newPass[50];
    const char *filename = (strcmp(role, "admin") == 0) ? ADMIN_FILE : STUDENT_FILE;
    FILE *file = fopen(filename, "r");
    FILE *tempFile = fopen("temp.txt", "w");
    int found = 0;

    if (file == NULL || tempFile == NULL) {
        printf("Error accessing user data.\n");
        return;
    }

    printf("\n=== Change Password ===\n");
    printf("Enter current password: ");
    getMaskedPassword(currentPass);

    while (fscanf(file, "%s %s %s %s", user.username, user.password,
           user.email, user.phone) != EOF) {
        if (strcmp(user.username, username) == 0) {
            found = 1;
            if (strcmp(user.password, currentPass) != 0) {
                printf("Incorrect current password!\n");
                fprintf(tempFile, "%s %s %s %s\n", user.username, user.password,
                       user.email, user.phone);
                break;
            }

            do {
                printf("Enter new password: ");
                getMaskedPassword(newPass);
            } while (!isStrongPassword(newPass, user.username));

            strcpy(user.password, newPass);
            printf("Password changed successfully!\n");
        }
        fprintf(tempFile, "%s %s %s %s\n", user.username, user.password,
               user.email, user.phone);
    }

    fclose(file);
    fclose(tempFile);

    if (found) {
        remove(filename);
        rename("temp.txt", filename);
    } else {
        remove("temp.txt");
        printf("User not found!\n");
    }
}

void viewBookReviews() {
    char isbn[20];
    printf("Enter ISBN of the book to see reviews: ");
    scanf("%s", isbn);

    FILE *reviewFile = fopen("reviews.txt", "r");
    if (!reviewFile) {
        printf("No reviews found for any book.\n");
        return;
    }

    char line[256], currentISBN[20], reviewer[50], reviewText[200];
    int rating, totalRatings = 0;
    float sumRatings = 0;

    printf("\n=== Reviews for Book (ISBN: %s) ===\n", isbn);
    printf("----------------------------------------\n");

    // Read all reviews and filter by ISBN
    while (fgets(line, sizeof(line), reviewFile)) {
        sscanf(line, "%[^|]|%[^|]|%d|%[^\n]",
               currentISBN, reviewer, &rating, reviewText);

        if (strcmp(currentISBN, isbn) == 0) {
            printf("Reviewer: %s\n", reviewer);
            printf("Rating: %d/5\n", rating);
            printf("Review: %s\n", reviewText);
            printf("----------------------------------------\n");

            sumRatings += rating;
            totalRatings++;
        }
    }

    // Display average rating if reviews exist
    if (totalRatings > 0) {
        printf("Average Rating: %.1f/5 (%d reviews)\n",
               sumRatings / totalRatings, totalRatings);
    } else {
        printf("No reviews found for this book.\n");
    }

    fclose(reviewFile);
}

void checkReservationNotifications(const char* username) {
    FILE *file = fopen("notifications.txt", "r");
    if (!file) return;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, username)) {
            printf("! %s", line); // Already includes newline
        }
    }
    fclose(file);
}

void viewNotifications(const char* username) {
    time_t now = time(NULL);
    struct IssuedBook issued;
    FILE *file = fopen(ISSUED_BOOK_FILE, "r");
    int hasNotifications = 0;

    if (file == NULL) {
        printf("\nNo notifications.\n");
        return;
    }

    printf("\n=== Notifications ===\n");

    while (fscanf(file, "%s %s %ld %ld %d", issued.username, issued.isbn,
           &issued.issueDate, &issued.dueDate, &issued.returned) != EOF) {
        if (strcmp(issued.username, username) == 0 && !issued.returned) {
            double secondsLeft = difftime(issued.dueDate, now);
            int daysLeft = secondsLeft / (60 * 60 * 24);

            if (now > issued.dueDate) {
                int daysOverdue = (now - issued.dueDate) / (60 * 60 * 24);
                printf("! Overdue: Book %s is %d days overdue. Fine: Taka %d\n",
                       issued.isbn, daysOverdue, daysOverdue * FINE_PER_DAY);
                hasNotifications = 1;
            } else if (daysLeft == 3 || daysLeft == 1) {
                printf("Reminder: Book %s is due in %d day(s).\n", issued.isbn, daysLeft);
                hasNotifications = 1;
            }
        }
    }

    fclose(file);

    checkReservationNotifications(username);

    if (!hasNotifications) {
        printf("No new notifications.\n");
    }
}

void reportIssue(const char* username) {
    char issue[200];
    printf("Describe your issue (max 200 characters): ");
    scanf(" %[^\n]%*c", issue);

    FILE *file = fopen("issues.txt", "a");
    if (!file) {
        printf("Error saving issue.\n");
        return;
    }

    fprintf(file, "%s|%s\n", username, issue);
    fclose(file);
    printf("Issue reported. Admin will review it soon.\n");
}

void manageFines() {
    FILE *paymentFile = fopen("payments.txt", "r");
    FILE *tempFile = fopen("temp_payments.txt", "w");
    FILE *notifFile = fopen("notifications.txt", "a");

    if (!paymentFile || !tempFile || !notifFile) {
        printf("Error accessing payment records.\n");
        return;
    }

    char line[256];
    char username[50], method[10], trxID[20], status[20];
    int amount;

    printf("\n=== Student Fine Payments ===\n");
    int anyPending = 0;

    while (fgets(line, sizeof(line), paymentFile)) {
        int fields = sscanf(line, "%[^|]|%[^|]|%d|%[^|]|%s", username, method, &amount, trxID, status);

        if (fields < 5) {
            // Backward compatibility
            strcpy(status, "pending");
        }

        if (strcmp(status, "pending") == 0) {
            anyPending = 1;
            printf("\nUsername: %s\nMethod: %s\nAmount: ₹%d\nTransaction ID: %s\n", username, method, amount, trxID);
            printf("1. Approve\n2. Reject\nChoice: ");
            int choice;
            scanf("%d", &choice);

            if (choice == 1) {
                // Do NOT write this to temp — it's approved and done
                fprintf(notifFile, "User: %s - Your payment of Taka %d was approved.\n", username, amount);
                printf("Payment approved.\n");
            } else {
                // Also don't keep it — it's rejected and notified
                fprintf(notifFile, "User: %s - Your payment of Taka %d was rejected. Please check your transaction ID.\n", username, amount);
                printf("Payment rejected.\n");
            }
        } else {
            // Already handled (approved/rejected) — don't re-show, don't re-write
            continue;
        }
    }

    fclose(paymentFile);
    fclose(tempFile);
    fclose(notifFile);

    // Replace file with only still-pending entries (if any)
    remove("payments.txt");
    rename("temp_payments.txt", "payments.txt");

    if (!anyPending) {
        printf("No pending payments found.\n");
    }
}

void viewAndManageIssues() {
    FILE *file = fopen("issues.txt", "r");
    FILE *temp = fopen("temp_issues.txt", "w");
    FILE *solved = fopen("solved_issues.txt", "a");

    if (!file || !temp || !solved) {
        printf("Error accessing issue records.\n");
        return;
    }

    char line[256], username[50], issue[200];
    int issueNumber = 0, found = 0;

    typedef struct {
        char username[50];
        char issue[200];
    } IssueEntry;

    IssueEntry issues[100];

    printf("\n=== Reported Issues ===\n");

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%[^|]|%[^\n]", username, issue) == 2) {
            found = 1;
            printf("\n[%d] From: %s\nIssue: %s\n", issueNumber + 1, username, issue);
            strcpy(issues[issueNumber].username, username);
            strcpy(issues[issueNumber].issue, issue);
            issueNumber++;
        }
    }

    if (!found) {
        printf("No unresolved issues.\n");
        fclose(file);
        fclose(temp);
        fclose(solved);
        remove("temp_issues.txt");
        return;
    }

    int choice;
    printf("\nEnter issue number to manage (0 to cancel): ");
    scanf("%d", &choice);

    if (choice < 1 || choice > issueNumber) {
        printf("Cancelled or invalid number.\n");
        fclose(file);
        fclose(temp);
        fclose(solved);
        remove("temp_issues.txt");
        return;
    }

    // Confirm solving
    int action;
    printf("1. Mark as Solved\n2. Leave as Unsolved\nChoice: ");
    scanf("%d", &action);

    rewind(file);
    int current = 0;
    while (fgets(line, sizeof(line), file)) {
        if (current == choice - 1) {
            if (action == 1) {
                fprintf(solved, "%s|%s\n", issues[current].username, issues[current].issue);
                printf("Marked as solved and removed from active issues.\n");
            } else {
                fputs(line, temp); // Keep it unresolved
                printf("Issue left unsolved.\n");
            }
        } else {
            fputs(line, temp);
        }
        current++;
    }

    fclose(file);
    fclose(temp);
    fclose(solved);

    remove("issues.txt");
    rename("temp_issues.txt", "issues.txt");
}

void viewAllStudents() {
    FILE *file = fopen(STUDENT_FILE, "r");
    struct User user;

    if (!file) {
        printf("No students registered yet.\n");
        return;
    }

    printf("\n=== Registered Students ===\n");
    printf("Username\t\tEmail\t\t\tPhone\n");
    printf("-----------------------------------------------------------\n");

    while (fscanf(file, "%s %s %s %s", user.username, user.password, user.email, user.phone) != EOF) {
        printf("%-16s\t%-24s\t%s\n", user.username, user.email, user.phone);
    }

    fclose(file);
}

void deleteStudent() {
    char target[50];
    printf("Enter the username of the student to delete: ");
    scanf("%s", target);

    FILE *file = fopen(STUDENT_FILE, "r");
    FILE *temp = fopen("temp_students.txt", "w");
    struct User user;
    int found = 0;

    if (!file || !temp) {
        printf("Error opening student file.\n");
        return;
    }

    while (fscanf(file, "%s %s %s %s", user.username, user.password, user.email, user.phone) != EOF) {
        if (strcmp(user.username, target) == 0) {
            found = 1;
            continue; // Skip this student (deleting)
        }
        fprintf(temp, "%s %s %s %s\n", user.username, user.password, user.email, user.phone);
    }

    fclose(file);
    fclose(temp);

    if (found) {
        remove(STUDENT_FILE);
        rename("temp_students.txt", STUDENT_FILE);
        printf("Student '%s' deleted successfully.\n", target);
    } else {
        remove("temp_students.txt");
        printf("No such student found.\n");
    }
}


void setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void printCover() {
    system("cls"); // Clear screen

    // Set colorful background and text
    setColor(15); // White text on blue background

    printf("\n\n");
setColor(11); // Light cyan
printf("\t\t\t\t  _____________________________________________\n");
printf("\t\t\t\t /                                             \\\n");
printf("\t\t\t\t|_______________________________________________|\n");
setColor(14); // Yellow
printf("\t\t\t\t|  ___  ___  ___  ___  ___  ___  ___  ___  ___  |\n");
printf("\t\t\t\t| |###||###||###||###||###||###||###||###||###| |\n");
printf("\t\t\t\t| |###||###||###||###||###||###||###||###||###| |\n");
setColor(12); // Light red
printf("\t\t\t\t| |###||###||###||###||###||###||###||###||###| |\n");
printf("\t\t\t\t| |___||___||___||___||___||___||___||___||___| |\n");
printf("\t\t\t\t|                                               |\n");
setColor(10); // Green
printf("\t\t\t\t|  ___  ___  ___  ___  ___  ___  ___  ___  ___  |\n");
printf("\t\t\t\t| |###||###||###||###||###||###||###||###||###| |\n");
printf("\t\t\t\t| |###||###||###||###||###||###||###||###||###| |\n");
setColor(13); // Pink
printf("\t\t\t\t| |###||###||###||###||###||###||###||###||###| |\n");
printf("\t\t\t\t| |___||___||___||___||___||___||___||___||___| |\n");
printf("\t\t\t\t|                                               |\n");
setColor(9); // Blue
printf("\t\t\t\t|  ___  ___  ___  ___  ___  ___  ___  ___  ___  |\n");
printf("\t\t\t\t| |###||###||###||###||###||###||###||###||###| |\n");
printf("\t\t\t\t| |###||###||###||###||###||###||###||###||###| |\n");
setColor(15); // White
printf("\t\t\t\t| |###||###||###||###||###||###||###||###||###| |\n");
printf("\t\t\t\t| |___||___||___||___||___||___||___||___||___| |\n");
printf("\t\t\t\t|_______________________________________________|\n");
printf("\t\t\t\t|                                               |\n");
setColor(14); // Yellow
printf("\t\t\t\t|       WELCOME TO LIBRARY MANAGEMENT SYSTEM    |\n");
printf("\t\t\t\t|_______________________________________________|\n\n");

    setColor(14); // Yellow
    printf("\t\t\t\t===================================================\n");
    setColor(10); // Green
    printf("\t\t\t\t          Developed by: TEAM: 08                   \n");
    setColor(11); // Light cyan
    printf("\t\t\t\t          Version: 2.1 (2025)                      \n");
    setColor(14);
    printf("\t\t\t\t===================================================\n\n");

    setColor(15); // White
    printf("\t\t\t\tPress any key to continue to the main menu...");
    getch();
    system("cls");
}

int main() {
    printCover();
    int choice;

    while (1) {
        printf("\nLibrary Management System\n");
        printf("1. Register as Admin\n");
        printf("2. Register as Student\n");
        printf("3. Login as Admin\n");
        printf("4. Login as Student\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                registerUser("admin");
                break;
            case 2:
                registerUser("student");
                break;
            case 3:
                if (loginUser("admin")) {
                    int adminChoice;
                    while (1) {
                        printf("\nAdmin Dashboard\n");
                        printf("1. Add New Book\n");
                        printf("2. Delete Book\n");
                        printf("3. View All Books\n");
                        printf("4. View All Students\n");
                        printf("5. Delete Student(s)\n");
                        printf("6. Issue Book\n");
                        printf("7. View All Issued Books\n");
                        printf("8. Search Books\n");
                        printf("9. Check Book Availability\n");
                        printf("10. Calculate Fine\n");
                        printf("11. Return Book\n");
                        printf("12. Manage Reported Issues\n");
                        printf("13. Manage Fine Payments\n");
                        printf("14. Logout\n");
                        printf("Enter your choice: ");
                        scanf("%d", &adminChoice);

                        switch (adminChoice) {
                            case 1: addBook(); break;
                            case 2: deleteBook(); break;
                            case 3: viewBooks(); break;
                            case 4: viewAllStudents(); break;
                            case 5: deleteStudent(); break;
                            case 6: issueBook(); break;
                            case 7: viewAllIssuedBooks(); break;
                            case 8: searchBooks(); break;
                            case 9: checkAvailability(); break;
                            case 10: calculateFine(); break;
                            case 11: returnBook(); break;
                            case 12: viewAndManageIssues(); break;
                            case 13: manageFines(); break;
                            case 14: printf("Logged out successfully.\n"); goto end_admin_dashboard;
                            default: printf("Invalid choice! Try again.\n");
                        }
                    }
                    end_admin_dashboard:;
                }
                break;
            case 4: {
    char *username = loginUser("student");
    if (username != NULL) {
        int studentChoice;
        while (1) {
            printf("\nStudent Dashboard - Welcome %s\n", username);
            printf("1. View All Books\n");
printf("2. Search Books\n");
printf("3. View Book Details\n");
printf("4. Request Book Issue\n");
printf("5. View Issued Books\n");
printf("6. Request Book Renewal/Return\n");
printf("7. View Borrowing History\n");
printf("8. View Notifications\n");
printf("9. Reserve a Book\n");
printf("10. Wishlist/Favorites\n");
printf("11. Rate/Review a Book\n");
printf("12. View Book Reviews\n");
printf("13. Pay Fines\n");
printf("14. Manage Profile\n");
printf("15. Change Password\n");
printf("16. Report Issues\n");
printf("17. Logout\n");
            printf("Enter your choice: ");
            scanf("%d", &studentChoice);

            switch(studentChoice) {
case 1: { // See All Books (Paginated)
    int page = 1, perPage = 5;
    FILE *bookFile = fopen(BOOK_FILE, "r");
    struct Book book;
    int count = 0;

    if (!bookFile) {
        printf("No books available.\n");
        break;
    }

    while (1) {
        printf("\n=== All Books (Page %d) ===\n", page);
        rewind(bookFile);
        int current = 0;
        while (fscanf(bookFile, "%s %s %s %s %d %d",
               book.isbn, book.title, book.author,
               book.genre, &book.totalCopies, &book.availableCopies) != EOF) {
            if (current >= (page-1)*perPage && current < page*perPage) {
                printf("\nISBN: %s\nTitle: %s\nAuthor: %s\nAvailable: %d/%d\n",
                       book.isbn, book.title, book.author,
                       book.availableCopies, book.totalCopies);
            }
            current++;
            count++;
        }

        printf("\n1. Next Page\n2. Previous Page\n3. Back\nChoice: ");
        int nav;
        scanf("%d", &nav);

        if (nav == 1 && page*perPage < count) page++;
        else if (nav == 2 && page > 1) page--;
        else if (nav == 3) break;
    }
    fclose(bookFile);
    break;
}
                case 2: searchBooksMenu(); break;
                case 3: viewBookDetails(); break;
                case 4: requestBookIssue(username); break;
                case 5: viewIssuedBooks(username); break;
                case 6: requestBookRenewalReturn(username); break;
                case 7: viewBorrowingHistory(username); break;
                case 8: viewNotifications(username); break;

case 9: {
    char isbn[20];
    printf("Enter ISBN to reserve: ");
    scanf("%s", isbn);

    // Check if book exists and is unavailable
    FILE *bookFile = fopen(BOOK_FILE, "r");
    struct Book book;
    int available = 0;
    while (fscanf(bookFile, "%s %s %s %s %d %d",
           book.isbn, book.title, book.author,
           book.genre, &book.totalCopies, &book.availableCopies) != EOF) {
        if (strcmp(book.isbn, isbn) == 0 && book.availableCopies == 0) {
            available = 1;
            break;
        }
    }
    fclose(bookFile);

    if (!available) {
        printf("Book is either available or doesn't exist.\n");
        break;
    }

    // Add reservation
    FILE *reserveFile = fopen("reservations.txt", "a");
    time_t now = time(NULL);
    time_t expiry = now + (2 * 24 * 60 * 60); // 2 days expiry
    fprintf(reserveFile, "%s|%s|%ld|%ld\n", username, isbn, now, expiry);
    fclose(reserveFile);

    printf("Reserved! You have 48 hours to collect when available.\n");
    break;
}


case 10: {
    printf("\n1. Add to Wishlist\n2. View Wishlist\nChoice: ");
    int wishChoice;
    scanf("%d", &wishChoice);

    if (wishChoice == 1) {
        char isbn[20];
        printf("Enter ISBN: ");
        scanf("%s", isbn);
        FILE *wishFile = fopen("wishlist.txt", "a");
        fprintf(wishFile, "%s|%s\n", username, isbn);
        fclose(wishFile);
        printf("Added to wishlist!\n");
    } else if (wishChoice == 2) {
        FILE *wishFile = fopen("wishlist.txt", "r");
        FILE *bookFile = fopen(BOOK_FILE, "r");
        char line[100], savedUser[50], savedISBN[20];
        struct Book book;

        if (!wishFile || !bookFile) {
            printf("Error opening wishlist or book file.\n");
            if (wishFile) fclose(wishFile);
            if (bookFile) fclose(bookFile);
            break;
        }

        printf("\n=== Your Wishlist ===\n");
        int foundAny = 0;
        while (fgets(line, sizeof(line), wishFile)) {
            sscanf(line, "%[^|]|%s", savedUser, savedISBN);
            if (strcmp(savedUser, username) == 0) {
                rewind(bookFile);
                int foundBook = 0;
                while (fscanf(bookFile, "%s %s %s %*s %*d %*d",
                       book.isbn, book.title, book.author) != EOF) {
                    if (strcmp(book.isbn, savedISBN) == 0) {
                        printf("- [%s] \"%s\" by %s\n", book.isbn, book.title, book.author);
                        foundBook = 1;
                        foundAny = 1;
                        break;
                    }
                }
                if (!foundBook) {
                    printf("- [%s] Book details not found.\n", savedISBN);
                    foundAny = 1;
                }
            }
        }

        if (!foundAny) {
            printf("Your wishlist is empty.\n");
        }

        fclose(wishFile);
        fclose(bookFile);
    } else {
        printf("Invalid choice!\n");
    }
    break;
}


case 11: {
    char isbn[20], review[200];
    int rating;
    printf("Enter ISBN to review: ");
    scanf("%s", isbn);
    printf("Rating (1-5): ");
    scanf("%d", &rating);
    printf("Review (max 200 chars): ");
    scanf(" %[^\n]%*c", review);

    FILE *reviewFile = fopen("reviews.txt", "a");
    fprintf(reviewFile, "%s|%s|%d|%s\n", isbn, username, rating, review);
    fclose(reviewFile);
    printf("Review submitted!\n");
    break;
}

case 12:
    viewBookReviews();
    break;

case 13: { // Pay Fines
    printf("\n=== Pay Fines ===\n");
    printf("1. bKash\n2. Nagad\nChoice: ");
    int payMethod;
    scanf("%d", &payMethod);

    char trxID[20];
    printf("Enter Transaction ID: ");
    scanf("%s", trxID);

    // In a real system, validate with API. Here we just log.
    FILE *paymentFile = fopen("payments.txt", "a");
    fprintf(paymentFile, "%s|%s|%d|%s\n", username,
            (payMethod == 1) ? "bKash" : "Nagad", FINE_PER_DAY, trxID);
    fclose(paymentFile);

    printf("Thank you. We will let you know soon if your payment was successful.\n");
    break;
}


                case 14: manageProfile(username); break;
                case 15: changePassword(username, "student"); break;
                case 16: reportIssue(username); break;
                case 17: printf("Logged out successfully.\n"); goto end_student_dashboard;
                default: printf("Invalid choice! Try again.\n");
            }
        }
        end_student_dashboard:;
    }
    break;
}
            case 5:
                printf("Exiting... Thank you!\n");
                exit(0);
            default:
                printf("Invalid choice! Try again.\n");
        }
    }

    return 0;
}
