#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEXT_FILE "students.txt"
#define BIN_FILE  "students.dat"
#define TEMP_BIN  "temp_students.dat"

typedef struct {
    int rollNo;
    char name[50];
    char department[40];
    int year;
    float marks;
    char grade;
} Student;

char calculateGrade(float marks);
void clearInputBuffer(void);
void syncTextFileFromBinary(const Student *records, int count);
int loadFromBinary(Student **records, int *count, int *capacity);
int saveAllToBinary(const Student *records, int count);
void addStudent(Student **records, int *count, int *capacity);
void displayAllStudents(const Student *records, int count);
void searchByRoll(const Student *records, int count);
void updateStudent(Student **records, int *count, int *capacity);
void deleteStudent(Student **records, int *count, int *capacity);
void countTotalStudents(int count);
void filterByMarks(const Student *records, int count);
void demonstrateCallocPreview(void);

char calculateGrade(float marks) {
    if (marks >= 90.0f) return 'A';
    else if (marks >= 75.0f) return 'B';
    else if (marks >= 60.0f) return 'C';
    else if (marks >= 50.0f) return 'D';
    else return 'F';
}

void clearInputBuffer(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
    }
}

void syncTextFileFromBinary(const Student *records, int count) {
    FILE *fp = fopen(TEXT_FILE, "w");
    if (fp == NULL) {
        perror("Error opening text file for writing");
        return;
    }

    fprintf(fp, "RollNo | Name | Department | Year | Marks | Grade\n");
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d|%s|%s|%d|%.2f|%c\n",
                records[i].rollNo,
                records[i].name,
                records[i].department,
                records[i].year,
                records[i].marks,
                records[i].grade);
    }

    fclose(fp);
}

int loadFromBinary(Student **records, int *count, int *capacity) {
    FILE *fp = fopen(BIN_FILE, "rb");
    if (fp == NULL) {
        *count = 0;
        *capacity = 2;
        *records = (Student *)malloc((*capacity) * sizeof(Student));
        if (*records == NULL) {
            printf("Initial malloc() failed!\n");
            return 0;
        }
        return 1;
    }

    *count = 0;
    *capacity = 4;
    *records = (Student *)malloc((*capacity) * sizeof(Student));
    if (*records == NULL) {
        printf("Memory allocation failed while loading records!\n");
        fclose(fp);
        return 0;
    }

    Student temp;
    while (fread(&temp, sizeof(Student), 1, fp) == 1) {
        if (*count >= *capacity) {
            int newCapacity = (*capacity) * 2;
            Student *resized = (Student *)realloc(*records, newCapacity * sizeof(Student));
            if (resized == NULL) {
                printf("realloc() failed while loading records; keeping existing data.\n");
                break;
            }
            *records = resized;
            *capacity = newCapacity;
        }
        (*records)[*count] = temp;
        (*count)++;
    }

    if (ferror(fp)) {
        printf("Error occurred while reading binary file.\n");
    }

    fclose(fp);
    return 1;
}

int saveAllToBinary(const Student *records, int count) {
    FILE *tempFp = fopen(TEMP_BIN, "wb");
    if (tempFp == NULL) {
        perror("Error creating temporary binary file");
        return 0;
    }

    if (count > 0) {
        size_t written = fwrite(records, sizeof(Student), (size_t)count, tempFp);
        if (written != (size_t)count) {
            printf("Error: Only %zu of %d records written to temporary file.\n", written, count);
            fclose(tempFp);
            remove(TEMP_BIN);
            return 0;
        }
    }

    if (fclose(tempFp) != 0) {
        perror("Error closing temporary binary file");
        remove(TEMP_BIN);
        return 0;
    }

    remove(BIN_FILE);
    if (rename(TEMP_BIN, BIN_FILE) != 0) {
        perror("Error updating binary data file");
        return 0;
    }

    syncTextFileFromBinary(records, count);
    return 1;
}

void demonstrateCallocPreview(void) {
    Student *zeroBuffer = (Student *)calloc(1, sizeof(Student));
    if (zeroBuffer == NULL) {
        printf("calloc() failed.\n");
        return;
    }
    free(zeroBuffer);
}

void addStudent(Student **records, int *count, int *capacity) {
    demonstrateCallocPreview();

    if (*count >= *capacity) {
        int newCapacity = (*capacity) * 2;
        Student *resized = (Student *)realloc(*records, newCapacity * sizeof(Student));
        if (resized == NULL) {
            printf("Memory reallocation failed! Cannot add new student right now.\n");
            return;
        }
        *records = resized;
        *capacity = newCapacity;
    }

    Student s;
    printf("\n--- Add Student Record ---\n");
    printf("Enter Roll Number: ");
    if (scanf("%d", &s.rollNo) != 1) {
        printf("Invalid roll number input.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    for (int i = 0; i < *count; i++) {
        if ((*records)[i].rollNo == s.rollNo) {
            printf("Error: A student with Roll Number %d already exists.\n", s.rollNo);
            return;
        }
    }

    printf("Enter Student Name: ");
    if (fgets(s.name, sizeof(s.name), stdin) == NULL) {
        printf("Error reading name.\n");
        return;
    }
    s.name[strcspn(s.name, "\n")] = '\0';

    printf("Enter Department: ");
    if (fgets(s.department, sizeof(s.department), stdin) == NULL) {
        printf("Error reading department.\n");
        return;
    }
    s.department[strcspn(s.department, "\n")] = '\0';

    printf("Enter Year (1-4): ");
    if (scanf("%d", &s.year) != 1 || s.year < 1 || s.year > 6) {
        printf("Invalid year entered.\n");
        clearInputBuffer();
        return;
    }

    printf("Enter Marks (0-100): ");
    if (scanf("%f", &s.marks) != 1 || s.marks < 0.0f || s.marks > 100.0f) {
        printf("Invalid marks entered.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    s.grade = calculateGrade(s.marks);

    (*records)[*count] = s;
    (*count)++;

    if (saveAllToBinary(*records, *count)) {
        printf("Student record added and saved (Grade: %c).\n", s.grade);
    } else {
        (*count)--;
        printf("Failed to save record.\n");
    }
}

void displayAllStudents(const Student *records, int count) {
    FILE *fp = fopen(TEXT_FILE, "r");
    if (fp == NULL) {
        printf("\nNo student records found (%s does not exist yet).\n", TEXT_FILE);
        return;
    }

    char headerLine[128];
    if (fgets(headerLine, sizeof(headerLine), fp) == NULL) {
        printf("\nText file is empty.\n");
        fclose(fp);
        return;
    }

    printf("\n%-8s %-22s %-18s %-6s %-8s %-6s\n",
           "RollNo", "Name", "Department", "Year", "Marks", "Grade");
    printf("------------------------------------------------------------------------\n");

    Student s;
    int found = 0;
    while (fscanf(fp, "%d|%49[^|]|%39[^|]|%d|%f|%c\n",
                  &s.rollNo, s.name, s.department, &s.year, &s.marks, &s.grade) == 6) {
        printf("%-8d %-22s %-18s %-6d %-8.2f %-6c\n",
               s.rollNo, s.name, s.department, s.year, s.marks, s.grade);
        found++;
    }

    if (found == 0) {
        printf("No records available to display (Total in memory: %d).\n", count);
    }

    fclose(fp);
}

void searchByRoll(const Student *records, int count) {
    int targetRoll;
    printf("\nEnter Roll Number to search: ");
    if (scanf("%d", &targetRoll) != 1) {
        printf("Invalid input.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    for (int i = 0; i < count; i++) {
        if (records[i].rollNo == targetRoll) {
            printf("\nRecord Found:\n");
            printf("Roll Number : %d\n", records[i].rollNo);
            printf("Name        : %s\n", records[i].name);
            printf("Department  : %s\n", records[i].department);
            printf("Year        : %d\n", records[i].year);
            printf("Marks       : %.2f\n", records[i].marks);
            printf("Grade       : %c\n", records[i].grade);
            return;
        }
    }

    printf("Student with Roll Number %d not found.\n", targetRoll);
}

void updateStudent(Student **records, int *count, int *capacity) {
    (void)capacity;
    int targetRoll;
    printf("\nEnter Roll Number to update: ");
    if (scanf("%d", &targetRoll) != 1) {
        printf("Invalid input.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    for (int i = 0; i < *count; i++) {
        if ((*records)[i].rollNo == targetRoll) {
            Student backup = (*records)[i];

            printf("Updating record for Roll Number %d (%s):\n", targetRoll, backup.name);
            printf("Enter New Name: ");
            if (fgets((*records)[i].name, sizeof((*records)[i].name), stdin) == NULL) {
                (*records)[i] = backup;
                return;
            }
            (*records)[i].name[strcspn((*records)[i].name, "\n")] = '\0';

            printf("Enter New Department: ");
            if (fgets((*records)[i].department, sizeof((*records)[i].department), stdin) == NULL) {
                (*records)[i] = backup;
                return;
            }
            (*records)[i].department[strcspn((*records)[i].department, "\n")] = '\0';

            printf("Enter New Year (1-4): ");
            if (scanf("%d", &(*records)[i].year) != 1 || (*records)[i].year < 1 || (*records)[i].year > 6) {
                printf("Invalid year. Update cancelled.\n");
                clearInputBuffer();
                (*records)[i] = backup;
                return;
            }

            printf("Enter New Marks (0-100): ");
            if (scanf("%f", &(*records)[i].marks) != 1 || (*records)[i].marks < 0.0f || (*records)[i].marks > 100.0f) {
                printf("Invalid marks. Update cancelled.\n");
                clearInputBuffer();
                (*records)[i] = backup;
                return;
            }
            clearInputBuffer();

            (*records)[i].grade = calculateGrade((*records)[i].marks);

            if (saveAllToBinary(*records, *count)) {
                printf("Record updated successfully.\n");
            } else {
                (*records)[i] = backup;
                printf("Failed to save updated record.\n");
            }
            return;
        }
    }

    printf("Student with Roll Number %d not found.\n", targetRoll);
}

void deleteStudent(Student **records, int *count, int *capacity) {
    int targetRoll;
    printf("\nEnter Roll Number to delete: ");
    if (scanf("%d", &targetRoll) != 1) {
        printf("Invalid input.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    int foundIndex = -1;
    for (int i = 0; i < *count; i++) {
        if ((*records)[i].rollNo == targetRoll) {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex == -1) {
        printf("Student with Roll Number %d not found.\n", targetRoll);
        return;
    }

    for (int i = foundIndex; i < (*count) - 1; i++) {
        (*records)[i] = (*records)[i + 1];
    }
    (*count)--;

    if (*count > 0 && *count <= (*capacity) / 2 && *capacity > 2) {
        int newCapacity = (*capacity) / 2;
        Student *shrunk = (Student *)realloc(*records, newCapacity * sizeof(Student));
        if (shrunk != NULL) {
            *records = shrunk;
            *capacity = newCapacity;
        }
    }

    if (saveAllToBinary(*records, *count)) {
        printf("Record with Roll Number %d deleted successfully.\n", targetRoll);
    } else {
        printf("Failed to update file after deletion.\n");
    }
}

void countTotalStudents(int count) {
    static int queryCount = 0;
    queryCount++;
    printf("\nTotal stored student records: %d (Checked %d time(s))\n", count, queryCount);
}

void filterByMarks(const Student *records, int count) {
    float minMarks;
    printf("\nEnter minimum marks threshold: ");
    if (scanf("%f", &minMarks) != 1) {
        printf("Invalid marks threshold.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    printf("\nStudents with marks greater than %.2f:\n", minMarks);
    printf("%-8s %-22s %-18s %-8s %-6s\n",
           "RollNo", "Name", "Department", "Marks", "Grade");
    printf("----------------------------------------------------------------\n");

    int matches = 0;
    for (int i = 0; i < count; i++) {
        if (records[i].marks > minMarks) {
            printf("%-8d %-22s %-18s %-8.2f %-6c\n",
                   records[i].rollNo,
                   records[i].name,
                   records[i].department,
                   records[i].marks,
                   records[i].grade);
            matches++;
        }
    }

    if (matches == 0) {
        printf("No students found with marks above %.2f.\n", minMarks);
    }
}

int main(void) {
    Student *records = NULL;
    int count = 0;
    int capacity = 0;

    if (!loadFromBinary(&records, &count, &capacity)) {
        return 1;
    }

    int choice;
    do {
        printf("\n=============================================\n");
        printf("  STUDENT RECORD & FILE MANAGEMENT SYSTEM\n");
        printf("=============================================\n");
        printf("1. Add Student Record\n");
        printf("2. Display All Records\n");
        printf("3. Search by Roll Number\n");
        printf("4. Update Student Record\n");
        printf("5. Delete Student Record\n");
        printf("6. Count Total Students\n");
        printf("7. Filter by Marks\n");
        printf("8. Exit\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input! Please enter a number from 1 to 8.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch (choice) {
            case 1:
                addStudent(&records, &count, &capacity);
                break;
            case 2:
                displayAllStudents(records, count);
                break;
            case 3:
                searchByRoll(records, count);
                break;
            case 4:
                updateStudent(&records, &count, &capacity);
                break;
            case 5:
                deleteStudent(&records, &count, &capacity);
                break;
            case 6:
                countTotalStudents(count);
                break;
            case 7:
                filterByMarks(records, count);
                break;
            case 8:
                printf("\nCleaning up memory and exiting...\n");
                break;
            default:
                printf("Invalid choice! Please select between 1 and 8.\n");
                break;
        }
    } while (choice != 8);

    free(records);
    records = NULL;

    return 0;
}

OUTPUT:

=============================================
  STUDENT RECORD & FILE MANAGEMENT SYSTEM
=============================================
1. Add Student Record
2. Display All Records
3. Search by Roll Number
4. Update Student Record
5. Delete Student Record
6. Count Total Students
7. Filter by Marks
8. Exit
Enter your choice: 1

--- Add Student Record ---
Enter Roll Number: 101
Enter Student Name: Kavya Nair
Enter Department: CSE
Enter Year (1-4): 1
Enter Marks (0-100): 88.5
Student record added and saved (Grade: B).

Enter your choice: 1

--- Add Student Record ---
Enter Roll Number: 102
Enter Student Name: Rohan Verma
Enter Department: ECE
Enter Year (1-4): 2
Enter Marks (0-100): 93.0
Student record added and saved (Grade: A).

Enter your choice: 2

RollNo   Name                   Department         Year   Marks    Grade 
------------------------------------------------------------------------
101      Kavya Nair             CSE                1      88.50    B     
102      Rohan Verma            ECE                2      93.00    A     

Enter your choice: 3

Enter Roll Number to search: 102

Record Found:
Roll Number : 102
Name        : Rohan Verma
Department  : ECE
Year        : 2
Marks       : 93.00
Grade       : A

Enter your choice: 4

Enter Roll Number to update: 101
Updating record for Roll Number 101 (Kavya Nair):
Enter New Name: Kavya Nair
Enter New Department: CSE
Enter New Year (1-4): 1
Enter New Marks (0-100): 91.0
Record updated successfully.

Enter your choice: 7

Enter minimum marks threshold: 90

Students with marks greater than 90.00:
RollNo   Name                   Department         Marks    Grade 
----------------------------------------------------------------
101      Kavya Nair             CSE                91.00    A     
102      Rohan Verma            ECE                93.00    A     

Enter your choice: 6

Total stored student records: 2 (Checked 1 time(s))

Enter your choice: 5

Enter Roll Number to delete: 101
Record with Roll Number 101 deleted successfully.

Enter your choice: 6

Total stored student records: 1 (Checked 2 time(s))

Enter your choice: 8

Cleaning up memory and exiting...