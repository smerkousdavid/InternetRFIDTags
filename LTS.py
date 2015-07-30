import sqlite3
import MySQLdb
import time
import config

from sys import exit


class MyDB(object):
    _conn = None
    _cur = None
    _last_id = None

    def __init__(self):
        self._conn = MySQLdb.connect(host=config.host,
                           user=config.user,
                           passwd=config.passwd,
                           db=config.db)
        self._cur = self._conn.cursor()
        self._last_id = self._cur.lastrowid


    def query(self, query, params=None):
        if params != None:
            final = self._cur.execute(query, params)
            self._last_id = self._cur.lastrowid
            return final
        else:
            final = self._cur.execute(query)
            self._last_id = self._cur.lastrowid
            return final


    def __del__(self):
        self._conn.close()




def wipearray():
    DB = MyDB()

    DB.query("DROP TABLE IF EXISTS learners")

    DB.query('''CREATE TABLE learners(
                `learner_id` INT AUTO_INCREMENT,
                `first_name` VARCHAR(45) NOT NULL,
                `last_name` VARCHAR(45) NOT NULL,
                `year` INT(3) NOT NULL DEFAULT 0,
                `notes` VARCHAR(255),
                `location` INT(3) NOT NULL DEFAULT 0,
                `check_in_time` INT(11) NOT NULL DEFAULT 0,
                `rfid` VARCHAR(36) NOT NULL DEFAULT 0,
                PRIMARY KEY (`learner_id`))''')


### I did this because in the bottom class, creating a learner requires inserting first_name and such else. In order to insert
### a name or whatever, you need an ID to insert it to. In order to get an ID, you need a name. Hence the issue
### Was there a better way of doing it? No doubt.
### Are there way more pressing issues? No doubt.
def createLearner(first_name,last_name):
    """Creates a learner and returns the ID of that learner
    :param first_name: First name (string)
    :param last_name: Last name (string)
    :return: ID of learner just created (long)
    """
    DB = MyDB()

    DB.query("""INSERT INTO learners
            SET first_name = %s,
            SET last_name = %s,
            SET year = 0,
            SET location = 0
            SET check_in_time = 0
            SET rfid = 0)""",
            params=(first_name,last_name))

    id = DB._last_id

    return int(id)

def learnerfromRFID(RFID):
    """ Creates a learner object
    :param RFID: RFID of learner (string)
    :return: Learner object (LTSBacked)
    """
    DB = MyDB()
    try:
        id = DB.query("SELECT learner_id FROM learners WHERE rfID = :rfID", {'rfID': RFID})
    except TypeError:
        raise ValueError('No learner with RFID:{}'.format(RFID))
    try:
        learner = LTSBackend(id)
        return learner
    except:
        # TODO : Add error/event logging for serious events like this
        print "Something went terribly, terribly wrong"
        exit()


### WTF PYTHON? Why do I need to put this stupid object parameter in order for setters to work?
class LTSBackend(object):

    def __init__(self, ID):
        # Grr this is a weird way of doing it
        # Basically the "Nones" are just telling python to execute the properties
        self.ID = ID
        self._first_name = None
        self._last_name = None
        self._Location = None
        self._RFID = None
        self._Notes = None

    @property
    def first_name(self):
        DB = MyDB()

        # TODO : Implement some form of checking before returning ID
        return DB.query("SELECT first_name FROM learners WHERE learner_id = %d", (self.ID))

    @first_name.setter
    def first_name(self,newName):
        DB = MyDB()

        DB.query("""UPDATE learners
                  SET first_name = %s
                  WHERE learner_id= %d""",
                 (newName,self.ID))

        # The reason I do the select is to make sure that it actually inserted it into the database
        # I should probably remove these to save speed.
        self._first_name = DB.query("SELECT first_name FROM learners WHERE learner_id = %d", (self.ID))

    @property
    def last_name(self):
        DB = MyDB()
        print "-----"
        print self.ID
        print type(self.ID)
        print "-----"

        return DB.query("SELECT last_name FROM learners WHERE learner_id = %d", (self.ID))

    @last_name.setter
    def last_name(self,newName):
        DB = MyDB()

        DB.query("""UPDATE learners
                SET last_name = %s
                WHERE learner_id = %d""",
                 (newName,self.ID))
        self._last_name = DB.query("SELECT last_name FROM learners WHERE learner_id = %d", (self.ID))

    @property
    def Notes(self):
        DB = MyDB()

        return DB.query("SELECT notes FROM learners WHERE learner_id = %d", (self.ID))

    @Notes.setter
    def Notes(self,newNotes):
        DB = MyDB()

        DB.query("""UPDATE learners
                  SET notes = %s
                  WHERE learner_id= %d""",
                 (newNotes, self.ID))

        self._Notes = DB.query("SELECT notes FROM learners WHERE learner_id = %d", (self.ID))

    @property
    def Location(self):
        DB = MyDB()

        return DB.query('SELECT location FROM learners WHERE learner_id= %d', (self.ID))

    @Location.setter
    def Location(self, location):
        DB = MyDB()

        # This writes in the time that the check in happens

        unix_time = time.time()
        DB.query("""UPDATE learners
                  SET check_in_time = %d
                  WHERE learner_id= %d""",
                 (unix_time, self.ID))
        DB.query("""UPDATE learners
                  SET location = %d
                  WHERE learner_id= %d""",
                 (location, self.ID))

        self._Location = DB.query('SELECT location FROM learners WHERE learner_id= %d', (self.ID))

    @property
    def RFID(self):
        DB = MyDB()

        return DB.query('SELECT rfid FROM learners WHERE learner_id= %d', (self.ID))

    @RFID.setter
    def RFID(self,newRFID):
        DB = MyDB()

        DB.query("""UPDATE learners
                SET rfID = %s
                WHERE learner_id= %d""",
                 (newRFID,self.ID))

        return DB.query('SELECT rfid FROM learners WHERE learner_id= %d', (self.ID))