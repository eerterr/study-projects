import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;

/**
 * The main class that implements the work of all program.
 * @author Daria Evdokimova
 * @version 23
 */
public class Main {
    /**{@value} is the maximum value of days*/
    public static final int MAX_DAYS = 30;
    /**{@value} is the maximum value of animals*/
    public static final int MAX_NUMBER_OF_ANIMALS = 20;
    /**{@value} is the value of index, which identify number of animals in array*/
    public static final int NUMBER_OF_ANIMALS_INDEX = 3;
    /**{@value} is the value of index, which identify type of animal in array*/
    public static final int TYPE_INDEX = 0;
    /**{@value} is the value of index, which identify weight of animal in array*/
    public static final int WEIGHT_INDEX = 1;
    /**{@value} is the value of index, which identify speed of animal in array*/
    public static final int SPEED_INDEX = 2;
    /**{@value} is the value of index, which identify energy of animal in array*/
    public static final int ENERGY_INDEX = 3;
    /**{@value} is the value of length of line with animal*/
    public static final int LENGTH_OF_LINE_WITH_ANIMAL = 4;

    /**
     * Main method in which program is running
     * @param args
     * @throws EnergyOutOfBoundsException
     * @throws SpeedOutOfBoundsException
     * @throws WeightOutOfBoundsException
     * @throws CannibalismException
     * @throws SelfHuntingException
     * @throws TooStrongPreyException
     * @throws FileNotFoundException
     * @throws GrassOutOfBoundsException
     */
    public static void main(String[] args) throws EnergyOutOfBoundsException, SpeedOutOfBoundsException,
            WeightOutOfBoundsException, CannibalismException, SelfHuntingException,
            TooStrongPreyException, FileNotFoundException, GrassOutOfBoundsException {
        //declaring an array into which we will read information from the file
        List<String> command = new ArrayList<>();
        //and read this information
        try {
            File inputFile = new File("input.txt");
            Scanner scanFile = new Scanner(inputFile);

            while (scanFile.hasNextLine()) {
                command.add(scanFile.nextLine());
            }
            scanFile.close();
        } catch (FileNotFoundException e) {
            System.out.println(e.getMessage());
            e.printStackTrace();
        }
        //check is it possible to get float value for writing it to grassAmount variable
        try {
            if (!isFloat(command.get(1))) {
                throw new InvalidInputsException();
            }
        } catch (InvalidInputsException e) {
            System.out.println(e.getMessage());
            System.exit(0);
        }
        //and write it
        float grassAmount = Float.parseFloat(command.get(1));
        Field field = new Field(grassAmount);
        //check is value of grassAmount is in bounds
        try {
            if (grassAmount > field.MAX_GRASS_AMOUNT || grassAmount < 0) {
                throw new GrassOutOfBoundsException();
            }
        } catch (GrassOutOfBoundsException e) {
            System.out.println(e.getMessage());
            System.exit(0);
        }
        //check is it possible to get float value for writing it to days variable
        try {
            if (!isFloat(command.get(0))) {
                throw new InvalidInputsException();
            }
        } catch (InvalidInputsException e) {
            System.out.println(e.getMessage());
            System.exit(0);
        }
        //and write it
        int days = Integer.parseInt(command.get(0));
        //check is value of days is in bounds
        try {
            if (days < 1 || days > MAX_DAYS) {
                throw new InvalidInputsException();
            }
        } catch (InvalidInputsException e) {
            System.out.println(e.getMessage());
            System.exit(0);
        }
        //read array with animals from file
        List<Animal> animals = readAnimals();
        //and immediately remove all already dead animals
        removeDeadAnimals(animals);
        //then launching the Method responsible for the execution of the simulation
        runSimulation(days, grassAmount, animals);
    }

    /**
     * Method, which simulates animals lives throughout all the days
     * @param days number of days
     * @param grassAmount amount of the grass
     * @param animals array with animals
     * @throws CannibalismException
     * @throws SelfHuntingException
     * @throws GrassOutOfBoundsException
     * @throws TooStrongPreyException
     */
    private static void runSimulation(int days, float grassAmount, List<Animal> animals)
            throws CannibalismException, SelfHuntingException, GrassOutOfBoundsException, TooStrongPreyException {
        int daysCounter = 1;
        Field field = new Field(grassAmount);
        while (daysCounter <= days) {
            for (Animal animal : animals) {
                if (animal.getEnergy() > 0) {
                    animal.eat(animals, field);
                }
            }
            for (Animal animal : animals) {
                animal.decrementEnergy();
            }
            removeDeadAnimals(animals);
            field.makeGrassGrow();
            daysCounter++;
        }
        //bring out sounds of the remaining animals
        printAnimals(animals);
    }

    /**
     * Method to write array with animals sfrom file
     * @return array with animals
     * @throws FileNotFoundException
     */
    private static List<Animal> readAnimals() throws FileNotFoundException {
        //declare empty array
        List<String> command = new ArrayList<>();
        //write into this array strings from file
        File inputFile = new File("input.txt");
        Scanner scanFile = new Scanner(inputFile);
        while (scanFile.hasNextLine()) {
            command.add(scanFile.nextLine());
        }
        scanFile.close();
        //check is it possible to get float value for writing it to number of animals variable
        try {
            if (!isInteger(command.get(2))) {
                throw new InvalidInputsException();
            }
        } catch (InvalidInputsException e) {
            System.out.println(e.getMessage());
            System.exit(0);
        }
        //and write it
        int numberOfAnimals = Integer.parseInt(command.get(2));
        //check is the number of animals is in bounds
        try {
            if (numberOfAnimals > MAX_NUMBER_OF_ANIMALS || numberOfAnimals < 1) {
                throw new InvalidInputsException();
            }
        } catch (InvalidInputsException e) {
            System.out.println(e.getMessage());
            System.exit(0);
        }
        //get a slice of the array, where written only animals
        List<String> stringsWithAnimals = command.subList(NUMBER_OF_ANIMALS_INDEX, command.size());
        //check whether the declared number of animals corresponds to their real number
        try {
            if (numberOfAnimals != stringsWithAnimals.size()) {
                throw new InvalidInputsException();
            }
        } catch (InvalidInputsException e) {
            System.out.println(e.getMessage());
            System.exit(0);
        }
        //declare empty array
        List<Animal> animals = new ArrayList<>();
        //and fill it out
        for (String s : stringsWithAnimals) {
            String[] lineInfo = s.split(" ");

            String type = lineInfo[TYPE_INDEX];
            //check is number of animal parameters is valid
            if (lineInfo.length != LENGTH_OF_LINE_WITH_ANIMAL) {
                try {
                    throw new InvalidNumberOfAnimalParametersException();
                } catch (InvalidNumberOfAnimalParametersException e) {
                    System.out.println(e.getMessage());
                    System.exit(0);
                }
            } else {
                //check is it possible to get float values for writing it to weight, speed and energy variables
                try {
                    if (!isFloat(lineInfo[WEIGHT_INDEX]) || !isFloat(lineInfo[SPEED_INDEX])
                            || !isFloat(lineInfo[ENERGY_INDEX])) {
                        throw new InvalidInputsException();
                    }
                } catch (InvalidInputsException e) {
                    System.out.println(e.getMessage());
                    System.exit(0);
                }
                //and write these values
                float weight = Float.parseFloat(lineInfo[WEIGHT_INDEX]);
                float speed = Float.parseFloat(lineInfo[SPEED_INDEX]);
                float energy = Float.parseFloat(lineInfo[ENERGY_INDEX]);

                if (type.equals("Boar")) {
                    animals.add(new Boar(weight, speed, energy));
                } else if (type.equals("Lion")) {
                    animals.add(new Lion(weight, speed, energy));
                } else if (type.equals("Zebra")) {
                    animals.add(new Zebra(weight, speed, energy));
                } else {
                    try {
                        throw new InvalidInputsException();
                    } catch (InvalidInputsException e) {
                        System.out.println(e.getMessage());
                        System.exit(0);
                    }
                }
                //check is value of weight is in bounds
                try {
                    if (weight < Animal.MIN_WEIGHT || weight > Animal.MAX_WEIGHT) {
                        throw new WeightOutOfBoundsException();
                    }
                } catch (WeightOutOfBoundsException e) {
                    System.out.println(e.getMessage());
                    System.exit(0);
                }
                //check is value of speed is in bounds
                try {
                    if (speed < Animal.MIN_SPEED || speed > Animal.MAX_SPEED) {
                        throw new SpeedOutOfBoundsException();
                    }
                } catch (SpeedOutOfBoundsException e) {
                    System.out.println(e.getMessage());
                    System.exit(0);
                }
                //check is value of energy is in bounds
                try {
                    if (energy < 0 || energy  > Animal.MAX_ENERGY) {
                        throw new EnergyOutOfBoundsException();
                    }
                } catch (EnergyOutOfBoundsException e) {
                    System.out.println(e.getMessage());
                    System.exit(0);
                }
            }
        }
        return animals;
    }

    /**
     * Method to removing dead animals
     * @param animals array with animals
     */
    private static void removeDeadAnimals(List<Animal> animals) {
        animals.removeIf(animal -> animal.getEnergy() <= 0);
    }

    /**
     * Methods to bring out sounds of all animals in array
     * @param animals array with animals
     */
    private static void printAnimals(List<Animal> animals) {
        for (Animal animal : animals) {
            Class<? extends Animal> animalClass = animal.getClass();
            animal.makeSound(animalClass);
        }
    }

    /**
     * Method to check is it possible to get integer value from string
     * @param strId string to check
     * @return true if it is possible and false otherwise
     */
    public static boolean isInteger(String strId) {
        try {
            float number = Integer.parseInt(strId);
            return true;
        } catch (NumberFormatException e) {
            return false;
        }
    }
    /**
     * Method to check is it possible to get float value from string
     * @param strId string to check
     * @return true if it is possible and false otherwise
     */
    public static boolean isFloat(String strId) {
        try {
            float number = Float.parseFloat(strId);
            return true;
        } catch (NumberFormatException e) {
            return false;
        }
    }
}

/**
 *Intarface with abstract functions for herbivore animals
 */
interface Herbivore {
    public void grazeInTheField(Animal grazer, Field field) throws GrassOutOfBoundsException;
}
/**
 *Intarface with abstract functions for carnivore animals
 */
interface Carnivore {
    public <T extends Animal> Animal choosePrey(List<Animal> animals, T hunter)
            throws SelfHuntingException, TooStrongPreyException, CannibalismException;
    public void huntPrey(Animal hunter, Animal prey);
}
/**
 *Intarface with abstract functions for omnivore animals
 */
interface Omnivore extends Carnivore, Herbivore { }

/**
 * Enumiration with animal sounds
 */
enum AnimalSound {
    LION("Roar"),
    ZEBRA("Ihoho"),
    BOAR("Oink");

    private String sound;

    AnimalSound(String sound) {
        this.sound = sound;
    }
    //getter
    public String getSound() {
        return sound;
    }
}

/**
 * Parent abstract class for all animals
 */
abstract class Animal {
    /**{@value} minimal value of animal weight*/
    public static final float MIN_WEIGHT = 5F;
    /**{@value} maximal value of animal weight*/
    public static final float MAX_WEIGHT = 200F;
    /**{@value} minimal value of animal speed*/
    public static final float MIN_SPEED = 5F;
    /**{@value} maximal value of animal speed*/
    public static final float MAX_SPEED = 60F;
    /**{@value} maximal value of animal energy*/
    public static final float MAX_ENERGY = 100F;
    private float weight;
    private float speed;
    private float energy;

    protected Animal(float weight, float speed, float energy) {
        this.weight = weight;
        this.speed = speed;
        this.energy = energy;
    }
    //getter functions
    public float getWeight() {
        return weight;
    }
    public float getSpeed() {
        return speed;
    }
    public float getEnergy() {
        return energy;
    }
    //setter for energy
    public void setEnergy(float energy) {
        if (energy > MAX_ENERGY) {
            this.energy = MAX_ENERGY;
        } else {
            this.energy = energy;
        }
    }
    public void decrementEnergy() {
        this.energy = energy - 1;
    }

    /**
     * Method, which displays sound for animal depends on animal class
     * @param animalClass class of animal
     */
    public void makeSound(Class<? extends Animal> animalClass) {
        if (animalClass == Boar.class) {
            System.out.println(AnimalSound.BOAR.getSound());
        }
        if (animalClass == Lion.class) {
            System.out.println(AnimalSound.LION.getSound());
        }
        if (animalClass == Zebra.class) {
            System.out.println(AnimalSound.ZEBRA.getSound());
        }
    }

    public abstract void eat(List<Animal> animals, Field field)
            throws CannibalismException, SelfHuntingException, TooStrongPreyException, GrassOutOfBoundsException;
}


class Field {
    /**{@value} maximal value of grass amount*/
    public static final int MAX_GRASS_AMOUNT = 100;
    private float grassAmount;

    public Field(float grassAmount) {
        this.grassAmount = grassAmount;
    }
    //getter for grass amount
    public float getGrassAmount() {
        return grassAmount;
    }
    //setter for grass amount
    public void setGrassAmount(float grassAmount) {
        this.grassAmount = grassAmount;
    }

    /**
     * Methods to make grass grows
     */
    public void makeGrassGrow() {
        grassAmount *= 2;
        if (grassAmount  >= MAX_GRASS_AMOUNT) {
            grassAmount = MAX_GRASS_AMOUNT;
        }
    }
}
/**
 * Child class of Animal
 */
class Lion extends Animal implements Carnivore {
    public Lion(float weight, float speed, float energy) {
        super(weight, speed, energy);
    }
    /**
     * Method, which let lion eats
     * @param animals array with animals
     * @param field
     */
    @Override
    public void eat(List<Animal> animals, Field field) {
        for (Animal animal : animals) {
            if (animal.equals(this)) {
                Animal prey = choosePrey(animals, this);
                huntPrey(this, prey);
            }
        }
    }
    /**
     * Method to choose prey from all array with animals for specifically animal
     * @param animals array with animals
     * @param hunter
     * @return prey for hanter
     * @param <T> useful for any animal types
     */
    @Override
    public <T extends Animal> Animal choosePrey(List<Animal> animals, T hunter) {
        int hunterIndex = -1;

        Animal prey = null;

        for (int i = 0; i < animals.size(); i++) {
            if (animals.get(i).equals(hunter)) {
                hunterIndex = i;
            }
        }
        if (hunterIndex == animals.size() - 1) {
            prey = animals.getFirst();
        } else {
            prey = animals.get(hunterIndex + 1);
        }
        return prey;
    }

    /**
     * Method to hunt prey
     * @param hunter
     * @param prey
     */
    @Override
    public void huntPrey(Animal hunter, Animal prey) {
        //check is it not self hunting
        try {
            if (hunter.equals(prey)) {
                throw new SelfHuntingException();
            }
        } catch (SelfHuntingException e) {
            System.out.println(e.getMessage());
            return;
        }
        //check is it not cannibalism
        try {
            if (prey instanceof Lion  && !hunter.equals(prey)) {
                throw new CannibalismException();
            }
        } catch (CannibalismException e) {
            System.out.println(e.getMessage());

            return;
        }
        //check is it possible to hunt prey
        try {
            if (hunter.getSpeed() <= prey.getSpeed() && hunter.getEnergy() <= prey.getEnergy()) {
                throw new TooStrongPreyException();
            }
        } catch (TooStrongPreyException e) {
            System.out.println(e.getMessage());
            return;
        }
        //kill prey and increasing the henter energy
        hunter.setEnergy(prey.getWeight() + hunter.getEnergy());
        prey.setEnergy(0);

    }
}
/**
 * Child class of Animal
 */
class Boar extends Animal implements Omnivore {
    public static final float WEIGHT_COEFFICIENT = 0.1F;

    public Boar(float weight, float speed, float energy) {
        super(weight, speed, energy);
    }
    /**
     * Method, which let boar eats
     * @param animals array with animals
     * @param field
     */
    @Override
    public void eat(List<Animal> animals, Field field) throws GrassOutOfBoundsException {
        for (Animal animal : animals) {
            if (animal.equals(this)) {
                grazeInTheField(this, field);
                Animal prey = choosePrey(animals, this);
                huntPrey(this, prey);
                break;
            }
        }
    }

    /**
     * Methods, which let herbivore animal eats
     * @param grazer
     * @param field
     * @throws GrassOutOfBoundsException
     */
    @Override
    public void grazeInTheField(Animal grazer, Field field) throws GrassOutOfBoundsException {
        if (field.getGrassAmount() > grazer.getWeight() * WEIGHT_COEFFICIENT) {
            grazer.setEnergy(grazer.getEnergy() + (grazer.getWeight() * WEIGHT_COEFFICIENT));
            field.setGrassAmount((float) (field.getGrassAmount() - grazer.getWeight() * WEIGHT_COEFFICIENT));
        }
    }
    /**
     * Method to choose prey from all array with animals for specifically animal
     * @param animals array with animals
     * @param hunter
     * @return prey for hanter
     * @param <T> useful for any animal types
     */
    @Override
    public <T extends Animal> Animal choosePrey(List<Animal> animals, T hunter) {
        int hunterIndex = -1;
        Animal prey = null;
        for (int i = 0; i < animals.size(); i++) {
            if (animals.get(i).equals(hunter)) {
                hunterIndex = i;
            }
        }
        if (hunterIndex == animals.size() - 1) {
            prey = animals.getFirst();
        } else {
            prey = animals.get(hunterIndex + 1);
        }
        return prey;
    }
    /**
     * Method to hunt prey
     * @param hunter
     * @param prey
     */
    @Override
    public void huntPrey(Animal hunter, Animal prey) {
        //check is it not self hunting
        try {
            if (prey.equals(hunter)) {
                throw new SelfHuntingException();
            }
        } catch (SelfHuntingException e) {
            System.out.println(e.getMessage());
            return;
        }
        //check is it not cannibalism
        try {
            if (prey instanceof Boar && !hunter.equals(prey)) {
                throw new CannibalismException();
            }
        } catch (CannibalismException e) {
            System.out.println(e.getMessage());
            return;
        }
        //check is it possible to hunt prey
        try {
            if ((hunter.getSpeed() <= prey.getSpeed()) && (hunter.getEnergy() <= prey.getEnergy())) {
                throw new TooStrongPreyException();
            }
        } catch (TooStrongPreyException e) {
            System.out.println(e.getMessage());
            return;
        }
        //kill prey and increasing the henter energy
        prey.setEnergy(0);
        hunter.setEnergy(prey.getWeight() + hunter.getEnergy());
    }
}

/**
 * Child class of Animal
 */
class Zebra extends Animal implements Herbivore {
    public static final float WEIGHT_COEFFICIENT = 0.1F;

    public Zebra(float weight, float speed, float energy) {
        super(weight, speed, energy);
    }
    /**
     * Method, which let zebra eats
     * @param animals array with animals
     * @param field
     */
    @Override
    public void eat(List<Animal> animals, Field field) {
        for (Animal animal : animals) {
            if (animal.equals(this)) {
                this.grazeInTheField(this, field);
            }
        }
    }
    /**
     * Methods, which let herbivore animal eats
     * @param grazer
     * @param field
     * @throws GrassOutOfBoundsException
     */
    @Override
    public void grazeInTheField(Animal grazer, Field field) {
        if (field.getGrassAmount() > grazer.getWeight() * WEIGHT_COEFFICIENT) {
            grazer.setEnergy(grazer.getEnergy() + (grazer.getWeight() * WEIGHT_COEFFICIENT));
            field.setGrassAmount((float) (field.getGrassAmount() - grazer.getWeight() * WEIGHT_COEFFICIENT));
        }
    }
}

class GrassOutOfBoundsException extends Throwable {
    public String getMessage() {
        return "The grass is out of bounds";
    }
}
class InvalidNumberOfAnimalParametersException extends Throwable {
    @Override
    public String getMessage() {
        return "Invalid number of animal parameters";
    }
}
class InvalidInputsException extends Throwable {
    public String getMessage() {
        return "Invalid inputs";
    }
}
class WeightOutOfBoundsException extends Throwable {
    public String getMessage() {
        return "The weight is out of bounds";
    }
}
class SpeedOutOfBoundsException extends Throwable {
    public String getMessage() {
        return "The speed is out of bounds";
    }
}
class EnergyOutOfBoundsException extends Throwable {
    public String getMessage() {
        return "The energy is out of bounds";
    }
}
class SelfHuntingException extends Throwable {
    public String getMessage() {
        return "Self-hunting is not allowed";
    }
}
class CannibalismException extends Throwable {
    public String getMessage() {
        return "Cannibalism is not allowed";
    }
}
class TooStrongPreyException extends Throwable {
    public String getMessage() {
        return "The prey is too strong or too fast to attack";
    }
}
