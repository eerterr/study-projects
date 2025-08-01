#include <ctype.h>
#include <stdio.h>
#include <string.h>

//introducing the necessary structures 
typedef struct {
    int student_id;
    char name[500],
    faculty[500];
} Student;

typedef enum {WRITTEN, DIGITAL} ExamType;

typedef union {
    int duration;
    char software[500];
} ExamInfo;

typedef struct {
    int exam_id;
    ExamType exam_type;
    ExamInfo exam_info;
} Exam;

typedef struct {
    int exam_id, student_id, grade;
} ExamGrade;

//checkers:

//checking the correspondence of string to enum elements
int str_in_enum(char s[]){
    if (strcmp(s, "WRITTEN")==0)  return WRITTEN;
    if (strcmp(s, "DIGITAL")==0)  return DIGITAL;
    return 3;
}

//checking the student's id for correctness
int check_student_id(int id){
    if (id>0 && id<1000) return 2;
    return 1;
}

//checking the exam's id for correctness
int check_exam_id(int id){
    if (id>0 && id<500) return 2;
    return 1;
}


//checking the length of the strings for compliance with the condition:

int check_lenth(char s[]){
    if (strlen(s) > 1 && strlen(s) < 20) return 2;
    return 1;
}

int check_lenth_faculty(char s[]){
    if (strlen(s) > 4 && strlen(s) < 30) return 2;
    return 1;
}

//checking the characters of the strings
int check_letters(char s[]){
    for (int i = 0; i < strlen(s); i++){
        if (isalpha(s[i]) == 0) return 1;
    }
    return 0;
}

//introducing arrays of structures
Student students[300];
Exam exams[300];
ExamGrade exam_grades[90000];

//and counters for the elements of these arrays
int student_number = 0;
int exam_number = 0;
int grade_number = 0;

//MAIN FUNCTIONS
void ADD_STUDENT(int id, char* name, char faculty[], FILE *fout) {

    //checking if there is already such a student
    for (int i = 0; i < student_number; i++){
        if (id == students[i].student_id){
            fprintf(fout, "Student: %d already exists\n", students[i].student_id);
            return;
            }
        }

    //checking the input data for correctness
    if (check_student_id(id) == 1){
            fprintf (fout, "%s", "Invalid student id\n");
            return;
        }
    if (check_lenth(name) == 1 || check_letters(name) != 0){
            fprintf (fout, "%s", "Invalid name\n");
            return;
    }
    if (check_lenth_faculty(faculty) == 1 || check_letters(faculty) != 0){
            fprintf (fout, "%s", "Invalid faculty\n");
            return;
        }
        
    //if everything is correct, we record the student's data
    students[student_number].student_id = id;
    strcpy(students[student_number].name, name);
    strcpy(students[student_number].faculty, faculty);

    fprintf(fout, "Student: %d added\n", students[student_number].student_id);
    student_number++;
    }

void ADD_EXAM (int id, char type_str[], FILE *fout, FILE *fin){

    ExamInfo info;

    //checking if there is already such an exam
    for (int i = 0; i < exam_number; i++){
        if (id == exams[i].exam_id){
            fprintf(fout, "Exam: %d already exists\n", exams[i].exam_id);
            return;
        }
    }

    //checking the input data for correctness
    if (check_exam_id(id) == 1){
            fprintf(fout, "%s", "Invalid exam id\n");
            return;
    }
    if (str_in_enum(type_str) == 3){
            fprintf(fout, "%s", "Invalid exam type\n");
            return;
    }

    //if type is correct write information about it
    if (str_in_enum(type_str) == WRITTEN){
        fscanf(fin, "%d", &info.duration);

        if (info.duration < 40 || info.duration > 180){
            fprintf(fout, "%s", "Invalid duration\n");
            return;
        }
    exams[exam_number].exam_info.duration = info.duration;
    }

    if (str_in_enum(type_str) == DIGITAL){
        fscanf(fin, "%s", info.software);

        if (check_letters(info.software)!=0 || strlen(info.software)<3 || strlen(info.software)>19){
                fprintf(fout, "%s", "Invalid software\n");
                return;
            }
            strcpy(exams[exam_number].exam_info.software, info.software);
    }

    //if everything is correct, we record the exam's data
    exams[exam_number].exam_id = id;
    exams[exam_number].exam_type = str_in_enum(type_str);
    fprintf(fout, "Exam: %d added\n", exams[exam_number].exam_id);
    exam_number++;
}

void ADD_GRADE (int exam_id, int student_id, int grade, FILE *fout, FILE *fin){

    //checking if there is an exam
    int cntr1 = 0;
    for(int i = 0; i < exam_number; i++){
        if (exam_id == exams[i].exam_id){
                cntr1+=1;
            }
    }

    if (cntr1==0){
        fprintf(fout, "%s", "Exam not found\n");
        return;
    }
            
    //and is its id correct
    if (check_exam_id(exam_id) == 1){
        fprintf(fout, "%s", "Invalid exam id\n");
            return;
        }

    //checking if there is an student
    int cntr2 = 0;
    for(int i = 0; i < student_number; i++){
        if (student_id == students[i].student_id){
            cntr2+=1;
        }
    }
    if (cntr2==0){
        fprintf(fout, "%s", "Student not found\n");
        return;
    }

    //and is his id correct
    if (check_student_id(student_id) == 1){
        fprintf(fout, "%s", "Invalid student id\n");
        return;
    }

    //checking grade is correct
    if (grade < 0 || grade > 100){
        fprintf(fout, "%s", "Invalid grade\n");
        return;
    }

    //if everything is correct, we record the grade's data
    exam_grades[grade_number].exam_id = exam_id;
    exam_grades[grade_number].student_id = student_id;
    exam_grades[grade_number].grade = grade;
    fprintf(fout, "Grade %d added for the student: %d\n",
    exam_grades[grade_number].grade, exam_grades[grade_number].student_id);
    grade_number++;
}

void SEARCH_STUDENT (int id, FILE *fout){

    //looking for the index of the right student
    int n = -1;
    for (int i = 0; i < student_number; i++){
        if (id == students[i].student_id){
            n = i;
            break;
        }
    }

    if (n == -1){
        fprintf(fout, "%s", "Student not found\n");
        return;
    }

    //checking is his id is correct
    if (check_student_id(id) == 1){
        fprintf(fout, "%s", "Invalid student id\n");
        return;
    }

    //if all is correct output the student's data according to the desired index
    fprintf(fout, "ID: %d, Name: %s, Faculty: %s\n",
    students[n].student_id, students[n].name, students[n].faculty);
}

void SEARCH_GRADE (int exam_id, int student_id, FILE *fout){

    //looking for the index of the right student and right exam
    int student_n = -1;
    for (int i = 0; i < student_number; i++){
        if (student_id == students[i].student_id){
            student_n = i;
            break;
        }
    }

    int exam_n = -1;
    for (int i = 0; i < exam_number; i++){
        if (exam_id == exams[i].exam_id){
            exam_n = i;
            break;
        }
    }

    if (exam_n == -1){
        fprintf(fout, "%s", "Exam not found\n");
        return;
    }

    if (student_n == -1){
        fprintf(fout, "%s", "Student not found\n");
        return;
    }

    //and check their id
    if (check_exam_id(exam_id) == 1){
        fprintf(fout, "%s", "Invalid exam id\n");
        return;
    }
            
    if (check_student_id(student_id) == 1 ){
        fprintf(fout, "%s", "Invalid student id\n");
        return;
    }

    //looking for the index of the right grade
    int grade_n = -1;
    for (int i = 0; i < grade_number; i++){
        if (exam_id == exam_grades[i].exam_id && student_id == exam_grades[i].student_id){
            grade_n = i;
            break;
        }
    }

    //and if all inputs are correct output the requested data considering exam type
    if (exams[exam_n].exam_type == WRITTEN){
        fprintf(fout,  "Exam: %d, Student: %d, Name: %s, Grade: %d, Type: WRITTEN, Info: %d\n",
        exams[exam_n].exam_id, students[student_n].student_id, students[student_n].name,
        exam_grades[grade_n].grade, exams[exam_n].exam_info.duration);
    }
    if (exams[exam_n].exam_type == DIGITAL){
        fprintf(fout,  "Exam: %d, Student: %d, Name: %s, Grade: %d, Type: DIGITAL, Info: %s\n",
        exams[exam_n].exam_id, students[student_n].student_id, students[student_n].name,
        exam_grades[grade_n].grade, exams[exam_n].exam_info.software);
        }
}

void UPDATE_EXAM (int id, char type_str[], FILE *fout, FILE *fin){

    ExamInfo info;
    //looking for the index of the exam witch we need to update
    int update_exam_number = 0;
    for (int i = 0; i < exam_number; i++){
        if (id == exams[i].exam_id){
            update_exam_number = i;
            break;
        }
    }

    //checking is exam type is correct
    if (str_in_enum(type_str) == 3){
        fprintf(fout, "%s", "Invalid exam type\n");
        return;
    }

    //and checking exam information
    //if all inputs are correct update the requested data considering exam type 
    if (str_in_enum(type_str) == WRITTEN){
        fscanf(fin, "%d", &info.duration);
        if (info.duration < 40 || info.duration > 180){
            fprintf(fout, "%s", "Invalid duration\n");
            return;
        }
    exams[update_exam_number].exam_type = WRITTEN;
    exams[update_exam_number].exam_info.duration = info.duration;
    }
    if (str_in_enum(type_str) == DIGITAL){
        fscanf(fin, "%s", info.software);
        if (check_letters(info.software) != 0 || strlen(info.software) < 3 || strlen(info.software) > 19 ){
            fprintf(fout, "%s", "Invalid software\n");
            return;
        }
    exams[update_exam_number].exam_type = DIGITAL;
    strcpy(exams[update_exam_number].exam_info.software, info.software);
    }
    fprintf(fout, "Exam: %d updated\n", exams[update_exam_number].exam_id);
}

void UPDATE_GRADE (int exam_id, int student_id, int new_grade, FILE *fout){

    //checking is grade is correct
    if (new_grade < 0 || new_grade > 100){
        fprintf(fout, "%s", "Invalid grade\n");
        return;
    }

    //looking for the index of the grade witch we need to update
    int update_grade_number = 0;
    for(int i = 0; i < grade_number; i++){
        if (student_id == exam_grades[i].student_id && exam_id == exam_grades[i].exam_id){
                update_grade_number = i;
        }
    }

    //if all inputs are correct update the requested data 
    exam_grades[update_grade_number].grade = new_grade;
    fprintf(fout, "Grade %d updated for the student: %d\n",
    exam_grades[update_grade_number].grade, exam_grades[update_grade_number].student_id);

}

void LIST_ALL_STUDENTS (FILE *fout){
    for (int i = 0; i < student_number; i++){
        SEARCH_STUDENT(students[i].student_id, fout);
    }
}

void DELETE_STUDENT (int id, FILE *fout){
    //moving all students after the deleted student for one position forward in the array
    for (int i = 0; i < student_number; i++) {
        if (id == students[i].student_id){
            for (int j = i; j < student_number - 1; j++){
                students[j] = students[j+1];
            }
        student_number--;
        break;
        }
    }
    //do the same with his grades
    int index = 0;
    while (index < grade_number) {
        if (exam_grades[index].student_id == id) {
            for (int i = index; i < grade_number; i++) {
                exam_grades[i] = exam_grades[i + 1];
            }
            grade_number--;
        }
        index++;
    }
    fprintf(fout, "Student: %d deleted\n", id);
}


int main(){

    //opening the files
    FILE* fin = fopen("input.txt", "r");
    FILE* fout = fopen("output.txt", "w");

    char str[100];
    
    //as long as there are elements in the file
    while (fscanf(fin, "%s", str) != EOF) {
        //compare the string with the names of the functions and execute the necessary ones
        if(strcmp(str, "ADD_STUDENT")==0) {
            int id;
            char name[100], faculty[100];
            fscanf(fin, "%d %s %s", &id, name, faculty);
            ADD_STUDENT (id, name, faculty, fout);
        }
        if (strcmp(str, "ADD_EXAM")==0){
            int id; char type_str[30];
            fscanf(fin, "%d %s", &id, type_str);
            ADD_EXAM (id, type_str, fout, fin);
        }
        if (strcmp(str, "ADD_GRADE")==0){
            int exam_id, student_id, grade;
            fscanf(fin, "%d %d %d", &exam_id, &student_id, &grade);
            ADD_GRADE (exam_id, student_id, grade, fout, fin);
        }
        if (strcmp(str, "SEARCH_STUDENT")==0){
            int id;
            fscanf(fin, "%d", &id);
            SEARCH_STUDENT (id, fout);
        }
        if (strcmp(str, "SEARCH_GRADE")==0){
            int exam_id, student_id;
            fscanf(fin, "%d %d", &exam_id, &student_id);
            SEARCH_GRADE(exam_id, student_id, fout);
        }
        if (strcmp(str, "UPDATE_EXAM")==0){
            int id; char type_str[30];
            fscanf(fin, "%d %s", &id, type_str);
            UPDATE_EXAM (id, type_str, fout, fin);
        }
        if (strcmp(str, "UPDATE_GRADE")==0){
            int exam_id, student_id, new_grade;
            fscanf(fin, "%d %d %d", &exam_id, &student_id, &new_grade);
            UPDATE_GRADE(exam_id, student_id, new_grade, fout);
        }
        if (strcmp(str, "LIST_ALL_STUDENTS")==0){
            LIST_ALL_STUDENTS (fout);
        }
        if (strcmp(str, "DELETE_STUDENT")==0){
            int id;
            fscanf(fin, "%d", &id);
            DELETE_STUDENT (id, fout);
        }
        if (strcmp(str, "END")==0){
            break;
        }
    }

    //closing the files
    fclose(fin);
    fclose(fout);

    return 0;
}
