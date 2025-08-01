#include <iostream>
#include <vector>
#include <sstream>
#include <memory>
#include <algorithm>

using namespace std;

//Parent Animal class according to UML diagram
class Animal {
protected:
    string name;
    int daysLived;
public:
    Animal() = default;
    Animal(string name, int days) : name(name), daysLived(days) {}
    virtual ~Animal() = default;

    int getDaysLived() const { return daysLived; }
    string getName() const { return name; }

    //show all information about animal, not only name but days to optimize some parts of code
    void sayName() const { cout << "My name is " << name << ", days lived: " << daysLived << endl ; }

    void setDaysLived(int days) { daysLived = days; }

    virtual void attack(Animal& other) = 0;
};

//Child class Fish according to UML diagram (inherit Animal)
class Fish : public virtual Animal {
public:
    Fish(string name, int days) : Animal(name, days) {}
    void attack(Animal& other) override { other.setDaysLived(0); }
};
//Child class BetterFish according to UML diagram (inherit Fish)
class BetterFish : public Fish {
public:
    BetterFish(string name, int days) : Animal(name, days), Fish(name, days) {}
    BetterFish(Fish& fish) :  Animal(fish.getName(), fish.getDaysLived()), Fish(fish.getName(), fish.getDaysLived()) {}

    void attack(Animal& other) override { other.setDaysLived(0); }
};
//Child class Mouse according to UML diagram (inherit Animal)
class Mouse : public virtual Animal {
public:
    Mouse(string name, int days) : Animal(name, days) {}
    void attack(Animal& other) override { other.setDaysLived(0); }
};
//Child class BetterMouse according to UML diagram (inherit Mouse)
class BetterMouse : public Mouse {
public:
    BetterMouse(string name, int days) : Animal(name,days), Mouse(name, days) {}
    BetterMouse(Mouse& mouse) : Animal(mouse.getName(), mouse.getDaysLived()), Mouse(mouse.getName(), mouse.getDaysLived()) {}

    void attack(Animal& other) override { other.setDaysLived(0); }
};
//Child class Bird according to UML diagram (inherit Animal)
class Bird : public virtual Animal {
public:
    Bird(string name, int days) : Animal(name, days) {}
    void attack(Animal& other) override { other.setDaysLived(0); }
};
//Child class BetterBird according to UML diagram (inherit Bird)
class BetterBird : public Bird {
public:
    BetterBird(string name, int days) : Animal(name, days), Bird(name, days) {};
    BetterBird(Bird& bird) :  Animal(bird.getName(), bird.getDaysLived()), Bird(bird.getName(), bird.getDaysLived()) {}

    void attack(Animal& other) override { other.setDaysLived(0); }
};
//Child class Monster according to UML diagram (inherit all BetterFish, BetterBird, and BetterMouse)
class Monster : public BetterFish, public BetterBird, public BetterMouse {
public:
    Monster (string name) : Animal(name, 1), BetterFish(name, 1), BetterBird(name, 1), BetterMouse(name, 1) {}
    Monster (Animal& animal)  : Animal(animal.getName(), 1), BetterFish(animal.getName(), 1), BetterBird(animal.getName(), 1), BetterMouse(animal.getName(), 1) {}

    void attack(Animal& other) override { other.setDaysLived(0); }
};
//Parent class for all containers
template <typename T>
class Container {
private:
    //comparator for sorting animals after some operations
    bool cmp(const unique_ptr<T> &current, const unique_ptr<T> &next) {

        //compare lived days, if their equal...
        if ((current->getDaysLived() != next->getDaysLived())) return (current->getDaysLived() < next->getDaysLived());

        //...compare letters in names and sort in by alphabet
        return current->getName() < next->getName();
    }
public:
    //vector with all animals placed in container
    vector<unique_ptr<T>> animals_in_container;

    //function to add animal into container
    void add_animal(unique_ptr<T> animal) {

        //add animal
        animals_in_container.push_back(move(animal));

        //sort updated vector with animals using comparator
        sort(animals_in_container.begin(), animals_in_container.end(), [this](const unique_ptr<T> &a, const unique_ptr<T> &b) {
            return this->cmp(a, b);
        });
    }

    //function to get out unique animal entity from the container
    unique_ptr<T> get_unique_animal(int index) {
        if (index < 0 || index >= animals_in_container.size()) return nullptr;
        unique_ptr<T> animal = move(animals_in_container[index]);

        //get out from container
        animals_in_container.erase(animals_in_container.begin() + index);

        //sort updated vector with animals using comparator
        sort(animals_in_container.begin(), animals_in_container.end(), [this](const unique_ptr<T> &a, const unique_ptr<T> &b) {
            return this->cmp(a, b);
        });

        //return unique animal entity
        return animal;
    }

    //function to remove animal from container
    void remove_animal(int index) {

        //removing
        animals_in_container.erase(animals_in_container.begin() + index);

        //sort updated vector with animals using comparator
        sort(animals_in_container.begin(), animals_in_container.end(), [this](const unique_ptr<T> &a, const unique_ptr<T> &b) {
            return this->cmp(a, b);
        });
    }
    //function to update days for all animals in container
    void update_days() {
        auto animal = animals_in_container.begin();
        while (animal != animals_in_container.end()) {

            //update number of lived days for each animal
            (*animal)->setDaysLived((*animal)->getDaysLived() + 1);

            //check if animal is monster
            auto* monster = dynamic_cast<Monster*>(animal->get());

            //if animal live more than 10 days or animal is monster make them died
            if ((*animal)->getDaysLived() > 10 || monster) {
                cout << (*animal)->getName() << " has died of old days" << endl;
                animal = animals_in_container.erase(animal);
            }
            //take next animal to checking
            else animal++;
        }
    }
};
//Child class Freedom (inherit Container)
template <typename A> class Freedom : public Container<A> {};
//Child class Cage (inherit Container)
template <typename T> class Cage : public Container<T> {};
//Child class Aquarium (inherit Container)
template <typename T> class Aquarium : public Container<T> {};

//universal talk function
template <typename T>
void talk (Container<T>& container, int index) {
    //check if index is invalid
    if (index >= container.animals_in_container.size()) {
        cout << "Animal not found" << endl;
        return;
    }
    //show information
    container.animals_in_container[index]->sayName();
}
//universal applying substance
template <typename T, typename B>
void apply_substance(Container<T>& container, Container<B>& better_container, int index) {
    //check if index is invalid
    if (index >= container.animals_in_container.size()) {
        cout<<"Animal not found"<<endl;
        return;
    }
    //get out animal from container
    unique_ptr<T> animal = container.get_unique_animal(index);
    //make it better
    auto better = make_unique<B>(animal->getName(), (animal->getDaysLived() + 1)/2);
    //put it in "better" container
    better_container.add_animal(move(better));
}
//universal applying substance for better animal
template <typename T, typename B>
void make_monster(Freedom<T>& freedom, Container<B>& better_container, int index) {
    //check if index is invalid
    if (index >= better_container.animals_in_container.size()) {
        cout<<"Animal not found"<<endl;
        return;
    }

    //get out animal from better container
    unique_ptr<B> better = better_container.get_unique_animal(index);
    //make monster
    auto monster = make_unique<Monster>(*better);

    //monster attack all animal in the same with him container
    for (int i = 0; i < better_container.animals_in_container.size(); i++) {
        monster->attack(*better_container.animals_in_container[i]);
        better_container.remove_animal(i);
    }
    //make monster free
    freedom.add_animal(move(monster));
}
//universal removing substance
template <typename T, typename B>
void remove_substance(Container<B>& better_container, Container<T>& container, int index) {
    //check if index is invalid
    if (index >= better_container.animals_in_container.size()) {
        cout<<"Animal not found"<<endl;
        return;
    }
    //get out animal from better container
    unique_ptr<B> better = better_container.get_unique_animal(index);
    //make it normal
    auto animal = make_unique<T>(better->getName(), better->getDaysLived()*2);
    //put it in "basic" container
    container.add_animal(move(animal));
}
//modified version of attack for optimize main function
template <typename T>
void attack_in_main (Container<T>& container, int index1, int index2) {
    //check if index is invalid
    if (index1 >= container.animals_in_container.size() || index2 >= container.animals_in_container.size()) {
        cout<<"Animal not found"<<endl;
        return;
    }
    //use animal's universal attack function
    container.animals_in_container[index1]->attack(*container.animals_in_container[index2]);
    //remove dead animal
    container.remove_animal(index2);
}

int main () {
    //make containers for all types of animal
    Cage<Bird> cageWithBirds;
    Cage<BetterBird> cageWithBetterBirds;
    Cage<Mouse> cageWithMice;
    Cage<BetterMouse> cageWithBetterMice;
    Aquarium<Fish> aquariumWithFishes;
    Aquarium<BetterFish> aquariumWithBetterFishes;
    Aquarium<Mouse> aquariumWithMice;
    Aquarium<BetterMouse> aquariumWithBetterMice;
    Freedom<Animal> freedom;

    //read number of commands
    int n;
    cin >> n;
    cin.ignore();

    //read commands
    for (int i = 0; i < n; i++) {
        string line;
        getline(cin, line);
        istringstream stream(line);
        vector<string> command;

        //read tokens of command until the end of the line
        string token;
        while (stream >> token) command.push_back(token);

        //check if line is empty
        if (command.empty()) break;

        //check the first token of command and make decision about which universal function should be applied

        //block for the creating an animsl
        if (command[0] == "CREATE"){
            //read number of days from command string
            int days = stoi(command[5]);

            //according to input data make decision about which type of animal should be created and in which container we should place it
            if (command[1].length() == 1) { //"M", "B", "F"
                switch(command[1][0]) {
                    case 'M': { //Mouse
                        auto mouse = make_unique<Mouse>(command[2], days);
                        switch (command[4][0]) {
                            case 'C' : //Cage
                                cageWithMice.add_animal(move(mouse)); break;
                            case 'A' : //Aquarium
                                aquariumWithMice.add_animal(move(mouse)); break;
                            default: //Freedom
                                freedom.add_animal(move(mouse)); break;
                        }
                        break;
                    }
                    case 'F' : { //Fish
                        auto fish = make_unique<Fish>(command[2], days);
                        (command[4] == "Aquarium") ? aquariumWithFishes.add_animal(move(fish)) : freedom.add_animal(move(fish));
                        break;
                    }
                    case 'B' : { //Bird
                        auto bird = make_unique<Bird>(command[2], days);
                        (command[4] == "Cage") ? cageWithBirds.add_animal(move(bird)) : freedom.add_animal(move(bird));
                        break;
                    }
                }
            }
            else { //"BM", "BB", "BF"
                switch(command[1][1]) {
                    case 'M': { //Mouse
                        auto better_mouse = make_unique<BetterMouse>(command[2], days);
                        switch (command[4][0]) {
                            case 'C' : //Cage
                                cageWithBetterMice.add_animal(move(better_mouse)); break;
                            case 'A' : //Aquarium
                                aquariumWithBetterMice.add_animal(move(better_mouse)); break;
                            default: //Freedom
                                freedom.add_animal(move(better_mouse)); break;
                        }
                        break;
                    }
                    case 'F' : { //Fish
                        auto better_fish = make_unique<BetterFish>(command[2], days);
                        (command[4] == "Aquarium") ? aquariumWithBetterFishes.add_animal(move(better_fish)) : freedom.add_animal(move(better_fish));
                        break;
                    }
                    case 'B' : { //Bird
                        auto better_bird = make_unique<BetterBird>(command[2], days);
                        (command[4] == "Cage") ? cageWithBetterBirds.add_animal(move(better_bird)) : freedom.add_animal(move(better_bird));
                        break;
                    }
                }
            }

            //print message with name of the animals and its amount of days lived
            cout << "My name is " << command[2] << ", days lived: " << days << endl;
        }

        //block for the substance applying
        else if (command[0] == "APPLY_SUBSTANCE"){
            //check if container is Freedom and take into account its specifics
            if (command[1] == "Freedom") {
                cout << "Substance cannot be applied in freedom" << endl;
                continue;
            }
            //read index from command string
            int index = stoi(command[3]);

            //according to input data make decision about which variables put into universal substance applying function
            if (command[2].length() == 1) { //"M", "B", "F"
                switch(command[2][0]) {
                    case 'M' : {
                        (command[1] == "Cage") ? apply_substance(cageWithMice, cageWithBetterMice, index) : apply_substance(aquariumWithMice, aquariumWithBetterMice, index);
                        break;
                    }
                    case 'F' : apply_substance(aquariumWithFishes, aquariumWithBetterFishes, index); break;
                    case 'B' : apply_substance(cageWithBirds, cageWithBetterBirds, index); break;
                }
            }
            else {
                switch(command[2][1]) { //"BM", "BB", "BF"
                    case 'M' : {
                        (command[1] == "Cage") ? make_monster(freedom, cageWithBetterMice, index) : make_monster(freedom, aquariumWithBetterMice, index);
                        break;
                    }
                    case 'F' : make_monster(freedom, aquariumWithBetterFishes, index); break;
                    case 'B' : make_monster(freedom, cageWithBetterBirds, index); break;
                }
            }
        }

        //block for the substance removing
        else if (command[0] == "REMOVE_SUBSTANCE"){
            //check if container is Freedom and take into account its specifics
            if (command[1] == "Freedom") {
                cout << "Substance cannot be removed in freedom" << endl;
                continue;
            }
            //read index from command string
            int index = stoi(command[3]);

            //according to input data make decision about which variables put into universal substance removing function
            if (command[2].length() == 2) { //"BM", "BB", "BF"
                switch(command[2][1]) {
                    case 'M' : {
                        (command[1] == "Cage") ? remove_substance(cageWithBetterMice, cageWithMice, index) : remove_substance(aquariumWithBetterMice, aquariumWithMice, index);
                        break;
                    }
                    case 'F' :  remove_substance(aquariumWithBetterFishes, aquariumWithFishes, index); break;
                    case 'B' : remove_substance(cageWithBetterBirds, cageWithBirds, index); break;
                }
            } else { //"M", "B", "F" or monster
                cout << "Invalid substance removal" << endl;
            }
        }

        //block for the attacking
        else if (command[0] == "ATTACK") {
            //check if container is Freedom and take into account its specifics
            if (command[1] == "Freedom") {
                cout << "Animals cannot attack in Freedom" << endl;
                continue;
            }
            //read indexes from command string
            int index1 = stoi(command[3]);
            int index2 = stoi(command[4]);

            //according to input data make decision about which variables put into universal attack modified function
            if (command[2].length() == 1) { //"M", "B", "F"
                switch(command[2][0]) {
                    case 'M' : { //Mouse
                        cout << "Mouse is attacking" << endl;
                        (command[1] == "Cage") ? attack_in_main(cageWithMice, index1, index2) : attack_in_main(aquariumWithMice, index1, index2);
                        break;
                    }
                    case 'F' : { //Fish
                        cout << "Fish is attacking" << endl;
                        attack_in_main(aquariumWithFishes, index1, index2);
                        break;
                    }
                    case 'B' : { //Bird
                        cout << "Bird is attacking" << endl;
                        attack_in_main(cageWithBirds, index1, index2);
                        break;
                    }
                }
            }
            else { //"BM", "BB", "BF"
                switch(command[2][1]) {
                    case 'M' : { //Mouse
                        cout << "BetterMouse is attacking" << endl;
                        (command[1] == "Cage") ? attack_in_main(cageWithBetterMice, index1, index2) : attack_in_main(aquariumWithBetterMice, index1, index2);
                        break;
                    }
                    case 'F' : { //Fish
                        cout << "BetterFish is attacking" << endl;
                        attack_in_main(aquariumWithBetterFishes, index1, index2);
                        break;
                    }
                    case 'B' : { //Bird
                        cout << "BetterBird is attacking" << endl;
                        attack_in_main(cageWithBetterBirds, index1, index2);
                        break;
                    }
                }
            }
        }

        //talk block
        else if (command[0] == "TALK"){
            //check if container is Freedom and take into account its specifics
            if (command[1] == "Freedom") {
                int index_for_free = stoi(command[2]);
                talk(freedom, index_for_free);
                continue;
            }
            //read index from command string
            int index = stoi(command[3]);

            //according to input data make decision about which variables put into universal talk modified function
            if (command[2].length() == 1) { //"M", "B", "F"
                switch (command[2][0]) {
                    case 'F' : talk(aquariumWithFishes, index); break;
                    case 'B' : talk(cageWithBirds, index); break;
                    case 'M' : (command[1] == "Cage") ? talk(cageWithMice, index) : talk(aquariumWithMice, index); break;
                }
            }
            else if (command[2].length() == 2) { //"BM", "BB", "BF"
                switch (command[2][1]) {
                    case 'F' : talk(aquariumWithBetterFishes, index); break;
                    case 'B' : talk(cageWithBetterBirds, index); break;
                    case 'M' : (command[1] == "Cage") ? talk(cageWithBetterMice, index) : talk(aquariumWithBetterMice, index); break;
                }
            }
        }

        else {//PERIOD
            //update days for each container
            cageWithBirds.update_days();
            cageWithBetterBirds.update_days();
            cageWithMice.update_days();
            cageWithBetterMice.update_days();
            aquariumWithFishes.update_days();
            aquariumWithBetterFishes.update_days();
            aquariumWithMice.update_days();
            aquariumWithBetterMice.update_days();
            freedom.update_days();
        }
    }
    return 0;
}
