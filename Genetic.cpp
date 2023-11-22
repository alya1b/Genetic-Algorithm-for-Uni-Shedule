#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <chrono>
#include <unordered_set>
#include <functional>

namespace std {
    template <>
    struct hash<std::tuple<int, int, bool>> {
        size_t operator()(const std::tuple<int, int, bool>& tup) const {
            auto [a, b, c] = tup;
            size_t hashVal = std::hash<int>{}(a);
            hashVal ^= std::hash<int>{}(b) + 0x9e3779b9 + (hashVal << 6) + (hashVal >> 2);
            hashVal ^= std::hash<bool>{}(c) + 0x9e3779b9 + (hashVal << 6) + (hashVal >> 2);
            return hashVal;
        }
    };
}

class Teacher;
class Course;

// Constants
constexpr int TEACHER_OVERWORKED_PENALTY = 1;
constexpr int TEACHER_SPLIT_PENALTY = 1;
constexpr int GROUP_SPLIT_PENALTY = 1;
constexpr float MUTATION_RATIO = 0.001;
constexpr int NUM_GENERATION = 1000;
constexpr int POPULATION = 200;
constexpr int TIME_SLOTS = 5*4;

class Group {
public:
    int id;
    std::string name;
    std::vector<Course> courses; // Changed to store pointers to Course objects

    Group(int _id, const std::string& _name, const std::vector<Course>& _courses)
        : id(_id), name(_name), courses(_courses) {}
};

class Course {
public:
    int id;
    std::string name;
    int lectureHours;
    int labHours;
    std::vector<Teacher> qualifiedTeachers; // Changed to store pointers to Teacher objects

    Course(int _id, const std::string& _name, int _lectureHours, int _labHours, const std::vector<Teacher>& _qualifiedTeachers)
        : id(_id), name(_name), lectureHours(_lectureHours), labHours(_labHours), qualifiedTeachers(_qualifiedTeachers) {}
};

class Teacher {
public:
    int id;
    std::string name;
    int maxHours;

    Teacher(int _id, std::string _name, int _maxHours) : id(_id), name(_name), maxHours(_maxHours) {}
};

class Meeting {
public:
    int teacherId;
    int groupId;
    bool isLecture;
    int meetingTime;
    int courseId;

    Meeting(int _teacherId, int _groupId,int _courseId, bool _isLecture, int _meetingTime)
        : teacherId(_teacherId), groupId(_groupId),courseId(_courseId), isLecture(_isLecture), meetingTime(_meetingTime) {}
};

class Data {
public:
    std::vector<Teacher> teachers;
    std::vector<Group> groups;
    std::vector<Course> courses;

  Data() {

    teachers.push_back(Teacher(0, "Teacher1", 20)); // ID, Name, Max Hours
    teachers.push_back(Teacher(1, "Teacher2", 20));
    teachers.push_back(Teacher(2, "Teacher3", 20));
    teachers.push_back(Teacher(3, "Teacher4", 20));
    teachers.push_back(Teacher(4, "Teacher5", 20));
    teachers.push_back(Teacher(5, "Teacher6", 20));


    Course course1(1, "Course1", 4, 2, {teachers[2], teachers[1]});
    Course course2(2, "Course2", 1, 1, {teachers[0], teachers[1]});
    Course course3(3, "Course3", 2, 2, {teachers[4]});
    Course course4(4, "Course4", 0, 1, {teachers[0]});
    Course course5(5, "Course5", 3, 0, {teachers[4], teachers[2]});
    Course course6(6, "Course6", 1, 1, {teachers[0], teachers[3]});
    Course course7(7, "Course7", 1, 2, {teachers[0], teachers[5]});
    Course course8(8, "Course8", 2, 0, {teachers[3], teachers[4]});
    Course course9(9, "Course9", 0 , 1, {teachers[0], teachers[5]});
    Course course10(10, "Course10", 2, 2, {teachers[3], teachers[5]});

    courses.push_back(course1);
    courses.push_back(course2);
    courses.push_back(course3);
    courses.push_back(course4);
    courses.push_back(course5);
    courses.push_back(course6);
    courses.push_back(course7);
    courses.push_back(course8);
    courses.push_back(course9);
    courses.push_back(course10);

    std::vector<Course> group1Courses = {course1, course2};
    Group group1(1, "Group1", group1Courses);

    std::vector<Course> group2Courses = {course1, course3, course4,course5, course6};
    Group group2(2, "Group2", group2Courses);

    std::vector<Course> group3Courses = {course1, course2, course3, course10};
    Group group3(3, "Group3", group3Courses);

    std::vector<Course> group4Courses = {course5, course6, course7, course8, course10};
    Group group4(4, "Group4", group4Courses);

    groups.push_back(group1);
    groups.push_back(group2);
    groups.push_back(group3);
    groups.push_back(group4);
}

};

class Schedule {
public:
    std::vector<Meeting> meetings;
    float fitnessValue;
    int numClasses;
    float numConflicts;

    Schedule(const Data &data) {

        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::srand(seed);

        fitnessValue = 0;
        numClasses = 0;
        numConflicts = 0;

        for (const auto& group : data.groups) {
            for (const auto& course : group.courses) {


                // Add lectures
                for (int i = 0; i < course.lectureHours; ++i) {
                    int randomTeacherIndex = rand() % course.qualifiedTeachers.size();
                    Teacher selectedTeacher = course.qualifiedTeachers[randomTeacherIndex];

                    int randomMeetingTime = rand() % TIME_SLOTS; 
                    Meeting meeting(selectedTeacher.id, group.id, course.id, true, randomMeetingTime);
                    meetings.push_back(meeting);
                    numClasses++;
                }

                // Add labs
                for (int i = 0; i < course.labHours; ++i) {
                    int randomTeacherIndex = rand() % course.qualifiedTeachers.size();
                    Teacher selectedTeacher = course.qualifiedTeachers[randomTeacherIndex];

                    int randomMeetingTime = rand() % TIME_SLOTS; 
                    Meeting meeting(selectedTeacher.id, group.id, course.id, false, randomMeetingTime);
                    meetings.push_back(meeting);
                    numClasses++;
                }

            }
        }
    }
    
    float calculateFitness(Data & data) {
        float conflicts = calculateConflicts(data);
        fitnessValue = 1 / (1 + conflicts);
        return fitnessValue;
    }

    int calculateConflicts(Data & data) {
        numConflicts = 0;

        std::unordered_map<int, std::unordered_set<int>> teacherLectureSlots;
        std::unordered_map<int, std::unordered_set<int>> teacherLabSlots;
        std::unordered_map<int, std::unordered_set<int>> groupSlots;
        std::unordered_map<int, int> teacherTeachingHours;
        
        for (const auto& meeting : meetings) {
            if (!groupSlots[meeting.groupId].insert(meeting.meetingTime).second) {
                numConflicts += GROUP_SPLIT_PENALTY;
            }
        }

        using SlotTuple = std::tuple<int, int, bool>;
        using TeacherSlotSet = std::unordered_set<SlotTuple>;
        using TeacherSlotsMap = std::unordered_map<int, TeacherSlotSet>;
        TeacherSlotsMap teacherSlots;

        for (const auto& meeting : meetings) {
            teacherTeachingHours[meeting.teacherId]++;
            auto& slotSet = teacherSlots[meeting.teacherId];


            if (meeting.isLecture) {
                bool isSameSubjectLecture = false;
                for (const auto& slot : slotSet) {

                    if (std::get<0>(slot) == meeting.courseId && std::get<1>(slot) == meeting.meetingTime) {
                        if (std::get<2>(slot) == meeting.isLecture){
                            isSameSubjectLecture = true;
                            teacherTeachingHours[meeting.teacherId]--;}
                        else {
                            numConflicts+=TEACHER_SPLIT_PENALTY; 
                            break;
                        }
                    }
                }
                if (!isSameSubjectLecture) {

                    slotSet.insert(std::make_tuple(meeting.courseId, meeting.meetingTime, meeting.isLecture));
                }
            } else {
                for (const auto& slot : slotSet) {

                    if (std::get<0>(slot) == meeting.courseId && std::get<1>(slot) == meeting.meetingTime) {
                        numConflicts+=TEACHER_SPLIT_PENALTY;
                         break;
                    }
                }
                slotSet.insert(std::make_tuple(meeting.courseId, meeting.meetingTime, meeting.isLecture));
            }
        }

        for (const auto& kv : teacherTeachingHours) {
            int teacherId = kv.first;
            int teachingHours = kv.second;

            if (teachingHours > data.teachers[teacherId].maxHours) {
                numConflicts += (teachingHours - data.teachers[teacherId].maxHours)*TEACHER_OVERWORKED_PENALTY;
            }
        }

        return numConflicts;
    }

    void print() const {
        for (const auto& meeting : meetings) {
            std::cout << "Teacher ID: " << meeting.teacherId
                      << ", Group ID: " << meeting.groupId
                      << ", Course ID: "<<meeting.courseId
                      << ", Is Lecture: " << (meeting.isLecture ? "Yes" : "No")
                      << ", Meeting Time: " << meeting.meetingTime << std::endl;
        }
    }
};

class Population {
public:
    int size;
    std::vector<Schedule> schedules;

    Population(){
        this->size = POPULATION;
    }

    Population(int _size, Data &data) : size(_size) {
        for (int i = 0; i < size; ++i) {
            Schedule schedule(data);
            schedules.push_back(schedule);
        }
    }
};

class GeneticAlgorithm {
public:
    Schedule crossover_schedule(const Schedule& schedule1, const Schedule& schedule2) {
        Schedule newSchedule(schedule1);

        for (size_t i = 0; i < schedule1.meetings.size(); ++i) {

            if (rand() % 2 == 0) {
                newSchedule.meetings[i] = schedule1.meetings[i];
            } else {
                newSchedule.meetings[i] = schedule2.meetings[i];
            }
        }

        return newSchedule;
    }
    Schedule mutate_schedule(const Schedule& schedule, const Data& data) {
        Schedule resultSchedule(data); 
        Schedule tempSchedule(data);

        for (size_t i = 0; i < schedule.meetings.size(); ++i) {
            if ((static_cast<float>(rand()) / RAND_MAX) < MUTATION_RATIO) {
                resultSchedule.meetings[i] = tempSchedule.meetings[i];
            }
            else resultSchedule.meetings[i] = schedule.meetings[i];
        }

        return resultSchedule;
    }

    void mutate_population(Population& population, const Data& data) {
        for (auto& schedule : population.schedules) {
            schedule = mutate_schedule(schedule, data);
        }
    }
    const int ELITISM_NUM = 5; 

    Population crossover_population(const Population& population) {
        Population newPopulation;


        std::vector<Schedule> sortedSchedules = population.schedules;
        std::sort(sortedSchedules.begin(), sortedSchedules.end(),
                  [](const Schedule& a, const Schedule& b) {
                      return a.fitnessValue > b.fitnessValue;
                  });

        // Select the best schedules (elites)
        for (int i = 0; i < ELITISM_NUM; ++i) {
            newPopulation.schedules.push_back(sortedSchedules[i]);
        }

        // Crossover to create the rest of the population
        while (newPopulation.schedules.size() < POPULATION) {
            // Select two schedules randomly from sortedSchedules for crossover
            int index1 = rand() % sortedSchedules.size();
            int index2 = rand() % sortedSchedules.size();

            Schedule childSchedule = crossover_schedule(sortedSchedules[index1], sortedSchedules[index2]);
            newPopulation.schedules.push_back(childSchedule);
        }

        return newPopulation;
    }

};

int main() {

    Data data; 

    Population population(POPULATION, data);

    GeneticAlgorithm geneticAlgo;

    for (int generation = 0; generation < NUM_GENERATION; ++generation) {
        for (auto& schedule : population.schedules) {
            schedule.calculateFitness(data);
        }

        Schedule bestSchedule = *std::max_element(population.schedules.begin(),
                                                    population.schedules.end(),
                                                      [](const Schedule& a, const Schedule& b) {
                                                          return a.fitnessValue < b.fitnessValue;
                                                      });
            Schedule worstSchedule = *std::max_element(population.schedules.begin(),
                                                    population.schedules.end(),
                                                      [](const Schedule& a, const Schedule& b) {
                                                          return a.fitnessValue > b.fitnessValue;
                                                      });
        if (bestSchedule.fitnessValue == 1 || generation==NUM_GENERATION-1){
            std::cout << "Generation: " << generation << std::endl;
                        std::cout << "Best Fitness: " << bestSchedule.fitnessValue << std::endl;
                        std::cout << "WOrst Fitness: " << worstSchedule.fitnessValue << std::endl;
                        std::cout << "Number of Classes: " << bestSchedule.numClasses << std::endl;
                        std::cout << "Number of Conflicts: " << bestSchedule.numConflicts << std::endl;
                        

                        // Print some meetings from the best schedule
                        std::cout << "Meetings of the Best Schedule:" << std::endl;
                        bestSchedule.print(); 

                        std::cout << "--------------------------------------" << std::endl;
            return 0;
        }
        
        if (generation % 10 == 0) {
            std::cout << "Generation: " << generation << std::endl;

            Schedule bestSchedule = *std::max_element(population.schedules.begin(),
                                                      population.schedules.end(),
                                                      [](const Schedule& a, const Schedule& b) {
                                                          return a.fitnessValue < b.fitnessValue;
                                                      });
            
            Schedule worstSchedule = *std::max_element(population.schedules.begin(),
                                                    population.schedules.end(),
                                                      [](const Schedule& a, const Schedule& b) {
                                                          return a.fitnessValue > b.fitnessValue;
                                                      });
            std::cout << "Best Fitness: " << bestSchedule.fitnessValue << std::endl;
            std::cout << "WOrst Fitness: " << worstSchedule.fitnessValue << std::endl;
            std::cout << "Number of Classes: " << bestSchedule.numClasses << std::endl;
            std::cout << "Number of Conflicts: " << bestSchedule.numConflicts << std::endl;

            std::cout << "--------------------------------------" << std::endl;
        }

        // Perform crossover on the population
        Population newPopulation = geneticAlgo.crossover_population(population);

        // Mutate the new population
        geneticAlgo.mutate_population(newPopulation, data);

        // Replace the old population with the new one
        population = newPopulation;
        
    }

    return 0;
}

