#include <iostream>
#include <memory>
#include <vector>
#include <stack>
#include <algorithm>
#include <unordered_map>
#include <windows.h>
#include <sstream>
#include <string>

using namespace std;
//forward declaration of classes for their usage in interfaces
class File;
class Directory;
//interface for future creation of SizeVisitor class
class Visitor {
public:
    //separate visit methods for file and directory classes
    virtual void visit(File &file) = 0;
    virtual void visit(Directory &directory) = 0;

    virtual ~Visitor() = default;
};
//base class for nodes (files and directories)
class Node {
public:
    int parent_id;
    string name;
    Node(int id, const string& name) : parent_id(id), name(name) {}
    virtual void accept(Visitor& v) = 0;
    virtual ~Node() = default;
};
//Flyweight for storing sets of file attributes
class FileProperties {
    bool read_only;
    string owner;
    string group;
    string extension;

public:
    FileProperties(bool r, const string& o, const string& g, const string& ext) : read_only(r), owner(o), group(g), extension(ext) {}

    string get_extension() const { return extension; }

    //comparator for use in unordered_map
    bool operator==(const FileProperties& other) const {
        return read_only == other.read_only && extension == other.extension && owner == other.owner && group == other.group;
    }
};
//factory for managing and creating the above flyweights
class FilePropertiesFactory {
    static unordered_map<string, shared_ptr<FileProperties>> properties;
public:
    static shared_ptr<FileProperties> get_properties(bool r, const string& owner, const string& group, const string& extension) {

        //store by key equal to the combined attribute string
        string key = to_string(r) + owner + group + extension;
        //if such a set of attributes already exists, return it...
        if (properties.find(key) != properties.end()) {
            return properties[key];
        }
        //...if not, create and add a new one
        shared_ptr<FileProperties> new_format = make_shared<FileProperties>(r, owner, group, extension);
        properties[key] = new_format;
        return new_format;
    }
};

//shared dictionary of all attribute sets
unordered_map<string, shared_ptr<FileProperties>> FilePropertiesFactory::properties;

//file class
class File : public Node {
    float size;
    //attributes stored in a flyweight
    shared_ptr<FileProperties> properties;
public:
    File(int id, bool r, const string &owner, const string &group, float size, const string &name, const string &extension) : Node(id, name), size(size) {
        properties = FilePropertiesFactory::get_properties(r, owner, group, extension);
    }
    //function to get extension, returns it from the flyweight
    string get_extension() {
        return properties->get_extension();
    }

    //function to get file size
    float get_size() { return size; }

    //override accept method from Visitor
    void accept(Visitor& v) override {
        v.visit(*this);
    }
    //override comparison operator "==" to allow use of find
    bool operator==(const File& other) const {
        return name == other.name && size == other.size && *(properties) == *(other.properties);
    }
};

//forward declaration of DFSIterator for use in directory functions
class DFSIterator;

//directory class
class Directory : public Node {
    int id;
public:
    vector<shared_ptr<Node>> children;

    //two constructors since directories can be created with two or three variables (directly inherited from root)
    Directory(int id, const string& name) : Node(0, name), id(id) {}
    Directory(int id, int parent_id, const string& name) : Node(parent_id, name), id(id) {}

    //forward declaration of function for creating an iterator
    unique_ptr <DFSIterator> createIterator(shared_ptr<Directory> self, const unordered_map<int, shared_ptr<Directory>>& directories);

    //getter for id
    int get_id() const { return id; }

    //function to add children to directory
    void add_child(shared_ptr<Node> node) { children.emplace_back(node); }

    //override accept function to use visitor
    void accept(Visitor& v) override {
        v.visit(*this);
        for (auto& child : children)
            child->accept(v);
    }
};

//interface for creating DFSIterator
class Iterator {
public:
    virtual bool has_next() = 0;
    virtual shared_ptr<Node> get_next() = 0;
    virtual ~Iterator() = default;
};

//class for working with the file tree (particularly for DFS output)
class DFSIterator : public Iterator {
    //stack to be used in DFS
    stack<shared_ptr<Node>> nodes_stack;
    //unordered_map to access all directories by id
    const unordered_map<int, shared_ptr<Directory>>& directories;

public:
    DFSIterator(shared_ptr<Directory> root, const unordered_map<int, shared_ptr<Directory>>& directories) : directories(directories) {
        nodes_stack.push(root);
    }

    //function to check for next node
    bool has_next() override { return !nodes_stack.empty(); }

    //function to get next node and traverse tree with DFS
    shared_ptr<Node> get_next() override {
        //if no next element, exit...
        if (!has_next()) return nullptr;

        //...if exists, pop it
        auto current = nodes_stack.top();
        nodes_stack.pop();

        //if node is a directory, push all its children into the stack
        if (auto directory = dynamic_pointer_cast<Directory>(current)) {
            for (auto x = directory->children.rbegin(); x != directory->children.rend(); ++x)
                nodes_stack.push(*x);
        }
        //return the current element
        return current;
    }

    //function to print the file tree
    void draw_tree(const shared_ptr<Directory>& node, const string& space = "", bool is_last = true, bool is_root = true) {

        //if the node is not root, draw edges depending on whether it's the last
        if (!is_root) {
            cout << space << (is_last ? "└── " : "├── ");
        }
        //print the node name
        cout << node->name << endl;

        //for all children of the node
        for (int i = 0; i < node->children.size(); ++i) {
            //get the child
            const auto& child = node->children[i];
            //check if it's the last child
            bool iss_last = (i == node->children.size() - 1);
            //depending on whether it's the last and whether it's the root, print either space or bar
            string new_space = space + (is_root ? "" : (is_last ? "    " : "│   "));

            //if the node is a directory...
            if (auto dir = dynamic_pointer_cast<Directory>(child)) {
                //...recursively call draw_tree
                draw_tree(dir, new_space, iss_last, false);
            }
                //if the node is a file...
            else if (auto file = dynamic_pointer_cast<File>(child)) {
                //print information in the desired format including edge rendering
                cout << new_space << (iss_last ? "└── " : "├── ") << file->name << "." << file->get_extension() << " (" << file->get_size() << "KB)" << endl;
            }
        }
    }
};
//now that we have defined the iterator, initialize the previously declared function in Directory
std::unique_ptr<DFSIterator> Directory::createIterator(shared_ptr<Directory> self, const unordered_map<int, shared_ptr<Directory>>& directories)  {
    return std::make_unique<DFSIterator>(self, directories);
}
//class for calculating total size of all files
class SizeVisitor : public Visitor {
    float common_size = 0;
    vector<File> visited;
public:
    SizeVisitor() = default;
    //override method for files...
    void visit(File &file) override {
        if (find(visited.begin(), visited.end(), file) == visited.end()) {
            //just add its size to the total
            common_size += file.get_size();
            visited.emplace_back(file);
        }
    }
    //...and separately for directories
    void visit(Directory &directory) override {
        for (auto& child : directory.children)
            //call the method for all its children
            child->accept(*this);
    }
    //function to get final total size
    float get_total_size() const {
        return common_size;
    }
};
//method to create an array from a string by splitting it by spaces
vector<string> str_to_arr(const string& str) {
    vector<string> result;
    stringstream ss(str);
    string token;
    while (ss >> token)
        result.push_back(token);
    return result;
}
//main function
int main() {
    //*to properly display edge symbols
    SetConsoleOutputCP(CP_UTF8);

    //declare and read number of nodes
    int n;
    cin>>n;
    cin.ignore();

    //define root
    shared_ptr<Directory> root = make_shared<Directory>(0, 0,".");
    //create map for storing all directories and accessing them by id
    unordered_map<int, shared_ptr<Directory>> directories;
    //add root to the map as the first element
    directories[0] = root;

    //temporary variable for storing nodes
    string temp_node;
    //read nodes based on their count
    for (int i = 0; i < n; i++) {
        getline(cin, temp_node);
        vector<string> node_info = str_to_arr(temp_node);
        //if the node is a Directory...
        if (node_info[0] == "DIR") {
            //depending on input length, create directory, link to parent and add to map
            if (node_info.size() == 3) {
                shared_ptr<Directory> directory = make_shared<Directory>(stoi(node_info[1]), node_info[2]);
                directories[stoi(node_info[1])] = directory;
                root->add_child(directory);
            } else {
                shared_ptr<Directory> directory = make_shared<Directory>(stoi(node_info[1]), stoi(node_info[2]),node_info[3]);
                directories[stoi(node_info[1])] = directory;
                directories[stoi(node_info[2])]->add_child(directory);
            }
        }
        //if the node is a File...
        else {
            //split name and extension
            vector<string> name_exepsion;

            stringstream ss(node_info[6]);
            string smth;

            while (getline(ss, smth, '.')) {
                name_exepsion.push_back(smth);
            }

            //create file
            shared_ptr<File> file = make_shared<File>(stoi(node_info[1]), node_info[2] == "T", node_info[3],
                                                      node_info[4], stof(node_info[5]), name_exepsion[0], name_exepsion[1]);
            //link it to its parent directory
            directories[stoi(node_info[1])]->add_child(file);
        }
    }

    //create SizeVisitor instance and use it to calculate total file size
    SizeVisitor size_visitor;
    root->accept(size_visitor);
    cout << "total: " << size_visitor.get_total_size() << "KB" << endl;

    //create iterator via method in Directory and use it to draw file tree
    auto iterator = root->createIterator(root, directories);
    iterator->draw_tree(root);

    return 0;
}
