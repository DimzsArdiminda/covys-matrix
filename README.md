
# ACTIVITY MANAGEMENT ALGORITHM BASED ON THE COVEY'S MATRIX CONCEPT

This project was built to complete the final exam assignment. This project is open source and can be used by anyone.

---

## 📖 About Project

This project is built using the **C++ programming language** and implements the **Covey’s Time Management Matrix** to help users prioritize their activities based on **importance** and **urgency**.

> _"The key is not to prioritize what's on your schedule, but to schedule your priorities."_ — Stephen R. Covey

### ✨ Features

- 🧠 **Time Management by Covey’s Matrix**
- 🖥️ **Fullscreen mode**
- 🌙 **Light/Dark mode toggle** (only available in GUI version)
- 🧮 **CLI version (console-based)** for simplicity and learning purposes

---

## 💻 Tech Stack

| Tool         | Description                           |
|--------------|---------------------------------------|
| C++          | Main programming language             |
| wxWidgets    | GUI Framework (planned/GUI version)   |
| Standard C++ | For console-based implementation      |

---

## 🗂️ Project Structure

```
activity-manager/
│
├── main.cpp               // Entry point
├── activity_manager.h     // Class declarations
├── activity_manager.cpp   // Class definitions
│
├── README.md              // Project documentation
├── contributing.md        // Contribution guide
├── LICENSE                // (Optional) License file
```

---

## 🚀 How to Run

### ⛓️ Compile Console Version (Linux/Mac/Windows)
Make sure you have `g++` installed.

```bash
g++ main.cpp activity_manager.cpp -o ./exe/aktivitas
./exe/aktivitas
```

You will be prompted to input a number of activities, and each will be categorized into one of the 4 quadrants based on urgency and importance.

---

## 📊 Quadrant Description

| Quadrant        | Description                           | Recommendation              |
|------------------|---------------------------------------|------------------------------|
| I                | **Important & Urgent**                | Do it now                    |
| II               | **Important but Not Urgent**          | Schedule for improvement     |
| III              | **Not Important but Urgent**          | Minimize or delegate         |
| IV               | **Not Important & Not Urgent**        | Eliminate or avoid if possible |

---

## 🤝 Contributing

Contributions are always welcome!  
See `contributing.md` for ways to get started.  
Please adhere to this project's `code of conduct`.

---

## 💬 Collaboration & Contact

- 📧 Email: [dimasardiminda@gmail.com](mailto:dimasardiminda@gmail.com)
