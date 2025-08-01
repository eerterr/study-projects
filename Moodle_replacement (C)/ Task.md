# Assigment 2. Moodle Replacement

**Time Limit:** 1 second  
**Memory Limit:** 256 MB  
**Input:** `input.txt`  
**Output:** `output.txt`

---

## Problem Description

You are tasked with building a **Course Management System** that manages students and their exams. Students may take various types of exams and their grades are to be stored.

---

## Data Structures

### `struct Student`
- `student_id`: `int` — Unique student ID
- `name`: `string` — Student's name
- `faculty`: `string` — Student's faculty

### `struct Exam`
- `exam_id`: `int` — Unique exam ID
- `exam_type`: `enum ExamType` — `WRITTEN` or `DIGITAL`
- `exam_info`: `union ExamInfo` — Info based on exam type

### `struct ExamGrade`
- `exam_id`: `int`
- `student_id`: `int`
- `grade`: `int`

### `enum ExamType`
- `WRITTEN`: pen & paper, needs `duration`
- `DIGITAL`: computer-based, needs `software`

### `union ExamInfo`
- `duration`: `int` — For written exams
- `software`: `string` — For digital exams

---

## Command Format

Read from `input.txt`  
Each line is a command:

```text
COMMAND [arg1] [arg2] ...
```

---

## Input Constraints

### Student
- `0 < student_id < 1000`
- `1 < len(name) < 20`, only letters
- `4 < len(faculty) < 30`, only letters

### Exam
- `0 < exam_id < 500`
- `WRITTEN`: `40 <= duration <= 180`
- `DIGITAL`: `2 < len(software) < 20`, only letters

### Grade
- `0 <= grade <= 100`

---

## Supported Commands & Responses

### `ADD_STUDENT [student_id] [name] [faculty]`

On success:
```
Student: student_id added
```

Errors (first matched, in priority):
- `Student: student_id already exists`
- `Invalid student id`
- `Invalid name`
- `Invalid faculty`

---

### `ADD_EXAM [exam_id] [exam_type] [exam_info]`

On success:
```
Exam: exam_id added
```

Errors:
- `Exam: exam_id already exists`
- `Invalid exam id`
- `Invalid exam type`
- `Invalid duration` (for WRITTEN)
- `Invalid software` (for DIGITAL)

---

### `ADD_GRADE [exam_id] [student_id] [grade]`

On success:
```
Grade grade added for the student: student_id
```

Errors:
- `Exam not found`
- `Invalid exam id`
- `Student not found`
- `Invalid student id`
- `Invalid grade`

---

### `UPDATE_EXAM [exam_id] [new_exam_type] [new_exam_info]`

On success:
```
Exam: exam_id updated
```

Errors:
- `Invalid exam type`
- `Invalid duration`
- `Invalid software`

> Note: It’s guaranteed that `exam_id` exists and is valid.

---

### `UPDATE_GRADE [exam_id] [student_id] [new_grade]`

On success:
```
Grade grade updated for the student: student_id
```

Errors:
- `Invalid grade`

> It’s guaranteed that `exam_id` and `student_id` are valid and exist.

---

### `SEARCH_STUDENT [student_id]`

On success:
```
ID: student_id, Name: name, Faculty: faculty
```

Errors:
- `Student not found`
- `Invalid student id`

---

### `SEARCH_GRADE [exam_id] [student_id]`

On success:
```
Exam: exam_id, Student: student_id, Name: name, Grade: grade, Type: exam_type, Info: exam_info
```

Errors:
- `Exam not found`
- `Invalid exam id`
- `Student not found`
- `Invalid student id`

---

### `LIST_ALL_STUDENTS`

Output:
List all students in the format:
```
ID: student_id, Name: name, Faculty: faculty
```
(one per line, in order of insertion)

---

### `DELETE_STUDENT [student_id]`

On success:
```
Student: student_id deleted
```

> Deletes student and all their exam grades.  
> `student_id` is guaranteed valid and existing.

---

### `END`

- Terminates processing.
- Do not print anything for this command.
- Ignore any following commands.

---

## Output Format

Write all responses to `output.txt`, one line per result.  
Always append a newline (`\n`) after each output line.

---

## Notes

- Input is **guaranteed syntactically valid** (correct command, argument count, space-separated).
- Output format must be followed exactly.
- Validate **value constraints** for each input field.

