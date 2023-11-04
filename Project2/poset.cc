#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include "poset.h"

namespace
{
  using std::cerr;
  using std::cin;
  using std::cout;
  using std::exit;
  using std::pair;
  using std::queue;
  using std::string;
  using std::to_string;
  using std::unordered_map;
  using std::vector;

  using posetID_t = unsigned long;
  using idx_t = size_t;
  using sizeOfPoset = size_t;
  using availableIDs = queue<posetID_t>;
  using posetElem = string;
  using relationType = int;
  using vectorOfStrings = vector<posetElem>;
  using posetRelationsArray = vector<vector<relationType>>;
  using poset_t = pair<vectorOfStrings *, posetRelationsArray *>;
  using allPosetsMap = unordered_map<posetID_t, poset_t *>;

  const int RELATION = 1;
  const int NO_RELATION = -1;
  const int REL_TRANSITIVITY = 2;

  /**
   * Preprocessor directive that checks whether NDEBUG flag is set.
   * It also creates bool constant which is later used in other functions
   * for checking whether they should print debugging messages.
   */ 
  #ifdef NDEBUG
  bool constexpr debug = false;
  #else
  bool constexpr debug = true;
  #endif

  /**
   * Macro for getting variables' names, it is needed for functions that print 
   * debugging messages. 
   */
  #define GET_VAR_NAME(x) #x

  /**
   * Function that solves order fiasko problem.
   * Creates static variable that stores all posets and
   * returns reference to it.
   */
  allPosetsMap &getAllPosets()
  {
    static allPosetsMap allPosets;
    return allPosets;
  }

  /**
   * Function that creates static variable (queue) that
   * stores available ids. It returns reference to it.
   */
  availableIDs &getAvailableIDs()
  {
    static availableIDs availableIDs;
    return availableIDs;
  }

  /**
   * Helper function for poset_del.
   * Checks if there are elements between elements at idx1=A and idx2=B
   * and in relation with them. Meaning if exists C that A < C < B.
   * If such C exists function returns true.
   */
  bool somethingIsBetweenTwoElem(posetRelationsArray *relationArr,
                                 idx_t idx1, idx_t idx2)
  {
    sizeOfPoset nbrOfRows = relationArr->size();

    for (idx_t i = 0; i < nbrOfRows; i++)
    {
      if (i != idx1 && i != idx2)
      {
        if ((relationArr->at(idx1)[i] == RELATION ||
             relationArr->at(idx1)[i] == REL_TRANSITIVITY) &&
            (relationArr->at(i)[idx2] == RELATION ||
             relationArr->at(i)[idx2] == REL_TRANSITIVITY))
        {
          return true;
        }
      }
    }

    return false;
  }

  /**
   * Helper function for poset_remove.
   * It finds elements that are in relation with  (smaller than) elem_to_remove,
   * meaning: elem1 < elem_to_remove
   * and if elem1 has transitivity relation with elem2,
   * where elem_to_remove < elem2, than relation between elem1 < elem2
   * is changed to normal relation.
   */
  void changeRelationForSmallerElem(posetRelationsArray *relArr,
                                    idx_t currentElem, idx_t idxOfElemToDelete, sizeOfPoset nbrOfRows)
  {
    for (idx_t j = 0; j < nbrOfRows; j++)
    {
      if ((relArr->at(idxOfElemToDelete)[j] == RELATION ||
           relArr->at(idxOfElemToDelete)[j] == REL_TRANSITIVITY) &&
          relArr->at(currentElem)[j] == REL_TRANSITIVITY)
      {
        relArr->at(currentElem)[j] = RELATION;
      }
    }
  }

  /**
   * Helper function for poset_remove.
   * It finds elements that are larger than elem_to_delete
   * meaning elem_to_delete < elem1. Than it finds elements that
   * elem2 < elem_to_delete and if elem2 < elem1 transitivity
   * then it changes the relation.
   */
  void changeRelationForLargerElem(posetRelationsArray *relArr,
                                   idx_t currentElem, idx_t idxOfElemToDelete, sizeOfPoset nbrOfRows)
  {
    for (idx_t j = 0; j < nbrOfRows; j++)
    {
      if ((relArr->at(j)[idxOfElemToDelete] == RELATION ||
           relArr->at(j)[idxOfElemToDelete] == REL_TRANSITIVITY) &&
          relArr->at(j)[currentElem] == REL_TRANSITIVITY)
      {
        relArr->at(j)[currentElem] = RELATION;
      }
    }
  }

  /**
   * Function checks if an element (value) is in a poset and sets
   * exist variable to adequate value, also if element exist it
   * remembers its position in vector in idx variable.
   */
  void checkIfElemExistInVecOfStr(vectorOfStrings *v, char const *value,
                                  idx_t &idx, bool &exist)
  {
    idx = 0;
    exist = false;
    sizeOfPoset vSize = v->size();

    for (idx_t i = 0; i < vSize; i++)
    {
      if ((*v)[i] == value)
      {
        exist = true;
        idx = i;
        break;
      }
    }
  }

  /**
   * Function knows that elemIdx1 < elemIdx2,
   * thus it adds transitivity relation between elems Xi < elemIdx1 and
   * elems Yi >= elemIdx2. Function does this by using nested for loop,
   * since it needs to check all elements in our poset.
   * If current Xi is in relation or transitivity relation
   * with elemIdx1 and is in neither of available relations
   * with elemIdx2 than Xi has transitivity relation with elemIdx2.
   * However, after we created this relation, we need to create
   * transitivity relations for Xi and all elems Yj that elemIdx2 <= Yj.
   */
  void addTransitivityRelations(posetRelationsArray *relationArr,
                                idx_t index1, idx_t index2)
  {
    sizeOfPoset nbrOfRows = relationArr->size();

    for (idx_t i = 0; i < nbrOfRows; i++)
    {
      if (i != index1 && i != index2)
      {
        // We check if Xi < elemIdx1 and if no rel between Xi and elemIdx2.
        if ((relationArr->at(i)[index1] == RELATION ||
             relationArr->at(i)[index1] == REL_TRANSITIVITY) &&
            relationArr->at(i)[index2] == NO_RELATION)
        {
          relationArr->at(i)[index2] = REL_TRANSITIVITY;

          // Now we add rel transitivity between Xi and Yj > elemIdx2. 
          for (idx_t j = 0; j < nbrOfRows; j++)
          {
            if ((relationArr->at(index2)[j] == RELATION ||
                 relationArr->at(index2)[j] == REL_TRANSITIVITY) &&
                relationArr->at(i)[j] == NO_RELATION)
            {
              relationArr->at(i)[j] = REL_TRANSITIVITY;
            }
          }
        }
        // Here we check if elemIdx1 is not in rel with Xi and whether
        // elemIdx2 < Xi, if it is we add rel between elemIdx1 and Xi.
        else if (relationArr->at(index1)[i] == NO_RELATION &&
                 (relationArr->at(index2)[i] == RELATION ||
                  relationArr->at(index2)[i] == REL_TRANSITIVITY))
        {
          relationArr->at(index1)[i] = REL_TRANSITIVITY;
        }
      }
    }
  }

  // FUNCTIONS THAT HANDLE DEBBUGING MESSAGES

  /**
   * Function creates string from char const* that is between "",
   * it also handles case when char const* is nullptr, and returns NULL.
   */
  string getStrErr(char const *val)
  {
    if (val == nullptr)
      return "NULL";

    string str(val);
    str = "\"" + str + "\"";
    return str;
  }

  /**
   * Function returns either string = (s1, s2) or
   * if s2 is empty it returns only (s1).
   */
  string getPairErr(string const &s1, string const &s2 = "")
  {
    if (!s2.empty())
      return "(" + s1 + ", " + s2 + ")";
    else
      return "(" + s1 + ")";
  }

  string getPosetIdErr(posetID_t id)
  {
    string str(": poset ");
    str = str + to_string(id);
    return str;
  }

  string lastExprErr(string s = "does not exist")
  {
    return " " + s + "\n";
  }

  string invalidValErr(char const *type)
  {
    string str(type);

    return ": invalid " + str + " (NULL)\n";
  }

  string commaElemErr(string s = "element")
  {
    return ", " + s + " ";
  }

  /**
   * Function returns string: "funcName: poset id, relation (val1, val2)".
  */
  string relationErr(string const &fName, posetID_t id,
                     char const *value1, char const *value2)
  {
    return fName + getPosetIdErr(id) + commaElemErr("relation") +
           getPairErr(getStrErr(value1), getStrErr(value2));
  }

  // FUNCTIONS THAT WRAP THE SAME DEBUGGING MESSAGES CASES

  /**
   * Function returns string: "funcName: poset id does not exist".
  */
  void posetNotExistErr(string const &fName, posetID_t id)
  {
    if constexpr (debug)
    {
      cerr << fName << getPosetIdErr(id) << lastExprErr();
    }
  }

  /**
   * Function returns string: 
   * "funcName: poset id, element "value" does not exist".
  */
  void elemNotExistErr(string const &fName, posetID_t id, char const *value)
  {
    if constexpr (debug)
    {
      cerr << fName << getPosetIdErr(id) << commaElemErr() << getStrErr(value) << lastExprErr();
    }
  }

  /**
   * Function returns string: 
   * "funcName: poset id, element "value" already exists".
  */
  void elemExistErr(string const &fName, posetID_t id, char const *value)
  {
    if constexpr (debug)
    {
      cerr << fName << getPosetIdErr(id) << commaElemErr() << getStrErr(value) << " already exists\n";
    }
  }

  /**
   * Function returns string: "funcName: invalid value (NULL)".
  */
  void oneValueNullErr(string const &fName, char const *value)
  {
    if constexpr (debug)
    {
      value = value;
      cerr << fName << invalidValErr(GET_VAR_NAME(value));
    }
  }

  /**
   * Function returns string: "funcName: invalid valuei (NULL)"
   * where valuei = value1 or value2 (meaning names of these variables).
  */
  void twoValueNullErr(string const &fName, char const *value1, 
    char const *value2)
  {
    if constexpr (debug)
    {
      if (value1 == nullptr)
        cerr << fName << invalidValErr(GET_VAR_NAME(value1));
      if (value2 == nullptr)
        cerr << fName << invalidValErr(GET_VAR_NAME(value2));
    }
  }

  /**
   * Function returns string: "funcName(id, "value1", "value2")".
  */
  void threeArgFuncNameErr(string const &fName, posetID_t id,
                           char const *value1, char const *value2)
  {
    if constexpr (debug)
    {
      cerr << fName << getPairErr(to_string(id), getStrErr(value1) + ", " + getStrErr(value2)) << "\n";
    }
  }

  /**
   * Function returns string: "funcName(id, "value")".
  */
  void twoArgFuncNameErr(string const &fName, posetID_t id, char const *value)
  {
    if constexpr (debug)
    {
      cerr << fName << getPairErr(to_string(id), getStrErr(value)) << "\n";
    }
  }

  /**
   * Function returns string: "funcName(id)".
   * If funcName = poset_new it returns "poset_new()".
  */
  void oneArgFuncNameErr(string const &fName, posetID_t id)
  {
    if constexpr (debug)
    {
      if (fName == "poset_new")
        cerr << fName << "()\n";
      else
        cerr << fName << getPairErr(to_string(id)) << "\n";
    }
  }

  /**
   * Function returns string: "funcName: poset id, element "value" inserted".
  */
  void insertedErr(string const &fName, posetID_t id, char const *value)
  {
    if constexpr (debug)
    {
      cerr << fName << getPosetIdErr(id) << commaElemErr() << getStrErr(value) << " inserted\n";
    }
  }

  /**
   * Function returns string: "funcName: poset id contains n element(s)".
  */
  void containsErr(string const &fName, posetID_t id, sizeOfPoset posetSize)
  {
    if constexpr (debug)
    {
      cerr << fName << getPosetIdErr(id) << " contains " << posetSize << 
      " element(s)\n";
    }
  }

  /**
   * Function returns string: "funcName: poset id, element "value" removed".
  */
  void elemRemovedErr(string const &fName, posetID_t id, char const *value)
  {
    if constexpr (debug)
    {
      cerr << fName << getPosetIdErr(id) << commaElemErr() << getStrErr(value) << " removed\n";
    }
  }

  /**
   * Function returns string: "funcName: poset id, relation (val1, val2) added"
   * or "... cannot be added". 
   * depending on value of isAdded variable.
  */
  void isRelationAddedErr(int isAdded, string const &fName, posetID_t id,
                          char const *value1, char const *value2)
  {
    if constexpr (debug)
    {
      if (isAdded == NO_RELATION)
      {
        cerr << relationErr(fName, id, value1, value2) << 
          lastExprErr("cannot be added");
      }
      else if (isAdded == RELATION)
      {
        cerr << relationErr(fName, id, value1, value2) << lastExprErr("added");
      }
    }
  }

  /**
   * Function returns string:"funcName: poset id, relation (val1, val2) deleted"
   * or "... cannot be deleted".
  */
  void isRelationDeletedErr(bool isDeleted, string const &fName, posetID_t id,
                            char const *value1, char const *value2)
  {
    if constexpr (debug)
    {
      if (isDeleted)
      {
        cerr << relationErr(fName, id, value1, value2) << 
          lastExprErr("deleted");
      }
      else
      {
        cerr << relationErr(fName, id, value1, value2) << 
          lastExprErr("cannot be deleted");
      }
    }
  }

  /**
   * Function returns string:"funcName: poset id, relation (val1, val2) exists"
   * or "... does not exist".
  */
  void relationExistsErr(bool exists, string const &fName, posetID_t id,
                         char const *value1, char const *value2)
  {
    if constexpr (debug)
    {
      if (exists)
      {
        cerr << relationErr(fName, id, value1, value2) << lastExprErr("exists");
      }
      else
      {
        cerr << relationErr(fName, id, value1, value2) << lastExprErr();
      }
    }
  }

  /**
   * Function returns string: "funcName: poset id expression",
   * where expression = created or cleared or deleted.
  */
  void stateOfPosetErr(string const &fName, posetID_t id)
  {
    if constexpr (debug)
    {
      string str;

      if (fName == "poset_new")
        str = "created";
      else if (fName == "poset_clear")
        str = "cleared";
      else
        str = "deleted";

      cerr << fName << getPosetIdErr(id) << lastExprErr(str);
    }
  }
}

namespace cxx
{
  /*
   * Creates a new poset and assigns it an id from the queue
   * of available (deleted) ids. If there are no such ids,
   * it assigns an id that is 1 greater than the previous one that
   * was not taken from queue.
   * Returns this id.
   */
  unsigned long poset_new(void)
  {
    static posetID_t nextID = 0;
    posetID_t id = 0;

    if constexpr (debug)
      oneArgFuncNameErr(string(__func__), id);

    auto &allPosets = getAllPosets();
    auto &availableIDs = getAvailableIDs();

    if (!availableIDs.empty())
    {
      id = availableIDs.front();
      availableIDs.pop();
    }
    else
    {
      id = nextID++;
    }

    poset_t *newPoset = new poset_t;
    vectorOfStrings *newVecOfPosetElem = new vectorOfStrings;
    posetRelationsArray *newRelationsArr = new posetRelationsArray;
    newPoset->first = newVecOfPosetElem;
    newPoset->second = newRelationsArr;

    allPosets.insert({id, newPoset});

    if constexpr (debug)
      stateOfPosetErr(string(__func__), id);

    return id;
  }

  /**
   * If a poset with the identifier id exists, function removes memory that
   * was allocated for the poset and adds its id to the queue of deleted itds.
   * Otherwise, it does nothing.
   */
  void poset_delete(unsigned long id)
  {
    if constexpr (debug)
      oneArgFuncNameErr(string(__func__), id);

    auto &allPosets = getAllPosets();
    auto it = allPosets.find(id);

    if (it != allPosets.end())
    {
      auto &availableIDs = getAvailableIDs();

      availableIDs.push(id);

      delete it->second->first;
      delete it->second->second;
      delete it->second;

      allPosets.erase(it);

      if constexpr (debug)
        stateOfPosetErr(string(__func__), id);
    }
    else
    {
      if constexpr (debug)
        posetNotExistErr(string(__func__), id);
    }
  }

  /*
   * If a poset with the indetifier id exists, the result
   * is the number of its elements.
   * Otherwise, it is 0.
   */
  size_t poset_size(unsigned long id)
  {
    if constexpr (debug)
      oneArgFuncNameErr(string(__func__), id);

    auto &allPosets = getAllPosets();
    sizeOfPoset sizeOfPoset = 0;
    auto it = allPosets.find(id);

    if (it != allPosets.end())
    {
      vectorOfStrings *v = it->second->first;
      sizeOfPoset = v->size();

      if constexpr (debug)
        containsErr(string(__func__), id, sizeOfPoset);
    }
    else
    {
      if constexpr (debug)
        posetNotExistErr(string(__func__), id);
    }

    return sizeOfPoset;
  }

  /*
   * If a poset with the indetifier id exists and the element value
   * is not part of that set, it adds the element to the set,
   * otherwise it does nothing. The new element is not related to any other
   * element. The result is true when the element has been added,
   * and false otherwise.
   */
  bool poset_insert(unsigned long id, char const *value)
  {
    if constexpr (debug)
      twoArgFuncNameErr(string(__func__), id, value);

    if (value == nullptr)
    {
      if constexpr (debug)
        oneValueNullErr(string(__func__), value);

      return false;
    }

    auto &allPosets = getAllPosets();
    auto it = allPosets.find(id);

    if (it != allPosets.end())
    {
      vectorOfStrings *v = it->second->first;

      for (const posetElem &str : *v)
      {
        if (str == value)
        {
          if constexpr (debug)
            elemExistErr(string(__func__), id, value);
          return false;
        }
      }
      
      posetElem elemToAdd(value);
      v->push_back(elemToAdd);

      posetRelationsArray *p = it->second->second;
      // Adding new row.
      p->push_back(vector<relationType>(p->size(), NO_RELATION));
      // Adding new column.
      for (vector<relationType> &row : *p)
        row.push_back(NO_RELATION);
      
      // An element is in relation with itself.
      p->at(p->size() - 1)[p->size() - 1] = RELATION;

      if constexpr (debug)
        insertedErr(string(__func__), id, value);

      return true;
    }
    else
    {
      if constexpr (debug)
        posetNotExistErr(string(__func__), id);
    }

    return false;
  }

  /*
   * If a poset with the identifier id exists and the element value
   * belongs to that set, it removes the element from the set and also
   * removes all relations of that element; otherwise, it does nothing.
   * The result is true when the element has been removed, and false otherwise.
   */
  bool poset_remove(unsigned long id, char const *value)
  {
    if constexpr (debug)
      twoArgFuncNameErr(string(__func__), id, value);

    if (value == nullptr)
    {
      if constexpr (debug)
        oneValueNullErr(string(__func__), value);
      return false;
    }

    bool elemExists = false;
    idx_t idxOfElemToDelete;
    auto &allPosets = getAllPosets();
    auto iter = allPosets.find(id);

    if (iter != allPosets.end())
    {
      vectorOfStrings *v = iter->second->first;

      checkIfElemExistInVecOfStr(v, value, idxOfElemToDelete, elemExists);

      if (elemExists)
      {
        (*v).erase((*v).begin() + idxOfElemToDelete);

        posetRelationsArray *relationArr = iter->second->second;
        sizeOfPoset nbrOfRows = relationArr->size();
        vector<relationType> *rowVec;

        for (idx_t i = 0; i < nbrOfRows; i++)
        {
          if (i != idxOfElemToDelete)
          {
            if (relationArr->at(i)[idxOfElemToDelete] == RELATION ||
                relationArr->at(i)[idxOfElemToDelete] == REL_TRANSITIVITY)
            {
              changeRelationForSmallerElem(relationArr, i, idxOfElemToDelete, nbrOfRows);
            }
            else if (relationArr->at(idxOfElemToDelete)[i] == RELATION ||
                     relationArr->at(idxOfElemToDelete)[i] == REL_TRANSITIVITY)
            {
              changeRelationForLargerElem(relationArr, i, idxOfElemToDelete, nbrOfRows);
            }
          }
        }

        for (idx_t i = 0; i < nbrOfRows; i++)
        {
          rowVec = &(relationArr->at(i));
          rowVec->erase(rowVec->begin() + idxOfElemToDelete);
        }

        relationArr->erase(relationArr->begin() + idxOfElemToDelete);

        if constexpr (debug)
          elemRemovedErr(string(__func__), id, value);

        return true;
      }
      else
      {
        if constexpr (debug)
          elemNotExistErr(string(__func__), id, value);
      }
    }
    else
    {
      if constexpr (debug)
        posetNotExistErr(string(__func__), id);
    }

    return false;
  }

  /*
   * If a poset with the identifier id exists and the elements value1
   * and value2 belong to that set and are not related, it extends
   * the relation so that the element value1 precedes the element value2;
   * otherwise, it does nothing.
   * The result is true when the relation has been extended,
   * and false otherwise.
   */
  bool poset_add(unsigned long id, char const *value1, char const *value2)
  {
    if constexpr (debug)
      threeArgFuncNameErr(string(__func__), id, value1, value2);

    if (value1 == nullptr || value2 == nullptr)
    {
      if constexpr (debug)
        twoValueNullErr(string(__func__), value1, value2);
      return false;
    }

    auto &allPosets = getAllPosets();
    auto it = allPosets.find(id);

    if (it != allPosets.end())
    {
      bool foundIndex1, foundIndex2;
      idx_t index1, index2;
      vectorOfStrings *v = it->second->first;

      checkIfElemExistInVecOfStr(v, value1, index1, foundIndex1);
      checkIfElemExistInVecOfStr(v, value2, index2, foundIndex2);

      if (!foundIndex1 || !foundIndex2)
      {
        if constexpr (debug)
        {
          char const *val;

          if (!foundIndex1)
            val = value1;
          else
            val = value2;

          if constexpr (debug)
            elemNotExistErr(string(__func__), id, val);
        }
        return false;
      }
      else
      {
        posetRelationsArray *relationArr = it->second->second;

        // If there is an edge between value1 and value2,
        // meaning the elements are in relation, then do nothing.
        if (relationArr->at(index1)[index2] == RELATION ||
            relationArr->at(index2)[index1] == RELATION ||
            relationArr->at(index1)[index2] == REL_TRANSITIVITY ||
            relationArr->at(index2)[index1] == REL_TRANSITIVITY)
        {
          if constexpr (debug)
            isRelationAddedErr(NO_RELATION, string(__func__), id, value1, value2);

          return false;
        }
        else
        {
          // Adding relation (edge between value1 and value2).
          relationArr->at(index1)[index2] = RELATION;

          // Now adds edges that will result from transitivity.
          addTransitivityRelations(relationArr, index1, index2);

          if constexpr (debug)
            isRelationAddedErr(RELATION, string(__func__), id, value1, value2);

          return true;
        }
      }
    }
    else
    {
      if constexpr (debug)
        posetNotExistErr(string(__func__), id);
    }

    return false;
  }

  /*
   * If a poset with the identifier id exists, the elements value1 and value2
   * belong to that set, element value1 precedes element value2, and removing
   * the relation between elements value1 and value2 will not violate
   * the conditions of being a partial order, then it removes the relation
   * between these elements; otherwise, it does nothing.
   * The result is true when the relation has been changed, and false otherwise.
   */
  bool poset_del(unsigned long id, char const *value1, char const *value2)
  {
    if constexpr (debug)
      threeArgFuncNameErr(string(__func__), id, value1, value2);

    if (value1 == nullptr || value2 == nullptr)
    {
      if constexpr (debug)
        twoValueNullErr(string(__func__), value1, value2);
      return false;
    }

    auto &allPosets = getAllPosets();
    auto iter = allPosets.find(id);

    if (iter != allPosets.end())
    {
      bool foundIdx1, foundIdx2;
      idx_t index1, index2;
      vectorOfStrings *v = iter->second->first;

      checkIfElemExistInVecOfStr(v, value1, index1, foundIdx1);
      checkIfElemExistInVecOfStr(v, value2, index2, foundIdx2);

      if (!foundIdx1 || !foundIdx2)
      {
        if constexpr (debug)
        {
          if (!foundIdx1)
            elemNotExistErr(string(__func__), id, value1);
          else
            elemNotExistErr(string(__func__), id, value2);
        }
        return false;
      }
      else if (index1 == index2)
      {
        if constexpr (debug)
          isRelationDeletedErr(false, string(__func__), id, value1, value2);

        return false;
      }
      else
      {
        posetRelationsArray *relationArr = iter->second->second;

        if (relationArr->at(index1)[index2] == RELATION ||
            relationArr->at(index1)[index2] == REL_TRANSITIVITY)
        {
          if (!somethingIsBetweenTwoElem(relationArr, index1, index2))
          {
            relationArr->at(index1)[index2] = NO_RELATION;
            relationArr->at(index2)[index1] = NO_RELATION;

            if constexpr (debug)
              isRelationDeletedErr(true, string(__func__), id, value1, value2);

            return true;
          }
        }
        if constexpr (debug)
          isRelationDeletedErr(false, string(__func__), id, value1, value2);
      }
    }
    else
    {
      if constexpr (debug)
        posetNotExistErr(string(__func__), id);
    }

    return false;
  }

  /*
   * If a poset with the identifier id exists, the elements value1 and value2
   * belong to that set, and element value1 precedes element value2,
   * then the result is true; otherwise, it is false."
   */
  bool poset_test(unsigned long id, char const *value1, char const *value2)
  {
    if constexpr (debug)
      threeArgFuncNameErr(string(__func__), id, value1, value2);

    if (value1 == nullptr || value2 == nullptr)
    {
      if constexpr (debug)
        twoValueNullErr(string(__func__), value1, value2);

      return false;
    }

    auto &allPosets = getAllPosets();
    auto it = allPosets.find(id);

    if (it != allPosets.end())
    {
      idx_t index1, index2;
      bool foundIdx1, foundIdx2;
      vectorOfStrings *v = it->second->first;

      checkIfElemExistInVecOfStr(v, value1, index1, foundIdx1);
      checkIfElemExistInVecOfStr(v, value2, index2, foundIdx2);

      if (!foundIdx1 || !foundIdx2)
      {
        if constexpr (debug)
        {
          if (!foundIdx1)
            elemNotExistErr(string(__func__), id, value1);
          else
            elemNotExistErr(string(__func__), id, value2);
        }
        return false;
      }
      else
      {
        posetRelationsArray *p = it->second->second;

        // If elements (value1 and value2) are in relation,
        // meaning if an element value1 precedes element value2.
        if (p->at(index1)[index2] == RELATION ||
            p->at(index1)[index2] == REL_TRANSITIVITY)
        {
          if constexpr (debug)
            relationExistsErr(true, string(__func__), id, value1, value2);

          return true;
        }
        if constexpr (debug)
          relationExistsErr(false, string(__func__), id, value1, value2);
      }
    }
    else
    {
      if constexpr (debug)
        posetNotExistErr(string(__func__), id);
    }

    return false;
  }

  /*
   * If the poset with the identifier id exists, it removes
   * all its elements and the relations between them.
   * Otherwise, it does nothing.
   */
  void poset_clear(unsigned long id)
  {
    if constexpr (debug)
      oneArgFuncNameErr(string(__func__), id);

    auto &allPosets = getAllPosets();
    auto it = allPosets.find(id);

    if (it != allPosets.end())
    {
      vectorOfStrings *v = it->second->first;
      posetRelationsArray *p = it->second->second;
      
      v->clear();
      p->clear();

      if constexpr (debug)
        stateOfPosetErr(string(__func__), id);
    }
    else
    {
      if constexpr (debug)
        posetNotExistErr(string(__func__), id);
    }
  }
}