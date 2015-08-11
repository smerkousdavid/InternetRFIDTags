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
        if params is not None:
            self._cur.execute(query, params)
            self._last_id = self._cur.lastrowid
            final = self._cur.fetchone()
            self._conn.commit()
            return final
        else:
            final = self._cur.execute(query)
            self._last_id = self._cur.lastrowid
            self._conn.commit()
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
def createLearner(first_name,last_name,year=None,rfid=None):
    """Creates a learner and returns the ID of that learner
    :param first_name: First name (string)
    :param last_name: Last name (string)
    :return: ID of learner just created (int)
    """
    DB = MyDB()
    create_query = ("INSERT INTO learners "
                    "(first_name,last_name,year,location,check_in_time,rfid) "
                    "VALUES (%(first_name)s,%(last_name)s,%(year)s,0,0,%(rfid)s)")
    create_data  = {
        'first_name' : first_name,
        'last_name'  : last_name,
        'year'       : 0 if year is None else year,
        'rfid'       : 0 if rfid is None else rfid,
        }

    DB.query(create_query,params=create_data)

    id = DB._last_id

    return int(id)

def learnerfromRFID(RFID):
    """ Creates a learner object
    :param RFID: RFID of learner (string)
    :return: Learner object (LTSBacked)
    """
    DB = MyDB()
    try:
        id = DB.query("SELECT learner_id FROM learners WHERE rfID = %s",RFID)[0]
    except TypeError:
        raise ValueError('No learner with RFID:{}'.format(RFID))
    learner = LTSBackend(id)
    return learner


### WTF PYTHON? Why do I need to put this stupid object parameter in order for setters to work?
class LTSBackend(object):

    def __init__(self, ID):
        # Grr this is a weird way of doing it
        # Basically the "Nones" are just telling python to execute the properties
        self.ID = ID
        self._first_name = None
        self._last_name = None
        self._Notes = None
        self._Location = None
        self._RFID = None


    @property
    def first_name(self):
        DB = MyDB()

        # TODO : Implement some form of checking before returning ID
        return DB.query("SELECT first_name FROM learners WHERE learner_id = %s", (self.ID))[0]

    @first_name.setter
    def first_name(self,new_name):
        DB = MyDB()

        DB.query("""UPDATE learners
                    SET first_name=%(new_name)s
                    WHERE learner_id=%(id)s""",
                    params={'new_name' : new_name,
                            'id'       : self.ID})

        # The reason I do the select is to make sure that it actually inserted it into the database
        # I should probably remove these to save speed.
        self._first_name = DB.query("SELECT first_name FROM learners WHERE learner_id = %s", (self.ID))[0]

    @property
    def last_name(self):
        DB = MyDB()
        return DB.query("SELECT last_name FROM learners WHERE learner_id = %(ID)s",{'ID' : 1})[0]

    @last_name.setter
    def last_name(self,new_name):
        DB = MyDB()

        DB.query("""UPDATE learners
                    SET last_name=%(new_name)s
                    WHERE learner_id=%(id)s""",
                    params={'new_name' : new_name,
                            'id'       : self.ID})

        self._last_name = DB.query("SELECT last_name FROM learners WHERE learner_id = %(ID)s",{'ID' : 1})[0]

    @property
    def Notes(self):
        DB = MyDB()

        return DB.query("SELECT notes FROM learners WHERE learner_id = %(ID)s",{'ID' : 1})[0]

    @Notes.setter
    def Notes(self,new_notes):
        DB = MyDB()

        DB.query("""UPDATE learners
                    SET notes=%(new_notes)s
                    WHERE learner_id=%(id)s""",
                    params={'new_notes' : new_notes,
                            'id'       : self.ID})

        self._Notes = DB.query("SELECT notes FROM learners WHERE learner_id = %(ID)s",{'ID' : 1})[0]

    @property
    def Location(self):
        DB = MyDB()

        return DB.query('SELECT location FROM learners WHERE learner_id= %s', (self.ID))[0]

    @Location.setter
    def Location(self, location):
        DB = MyDB()

        # check_in_time is in unixtime
        DB.query("""UPDATE learners
                  SET check_in_time = %(unix_time)s
                  WHERE learner_id= %(id)s""",
                  params={'unix_time' : time.time(),
                         'id'         : self.ID})
        DB.query("""UPDATE learners
                  SET location = %s
                  WHERE learner_id= %s""",
                 (location, self.ID))

        self._Location = DB.query('SELECT location FROM learners WHERE learner_id= %s', (self.ID))

    @property
    def RFID(self):
        DB = MyDB()

        return DB.query('SELECT rfid FROM learners WHERE learner_id= %s', (self.ID))[0]

    @RFID.setter
    def RFID(self,newRFID):
        DB = MyDB()

        DB.query("""UPDATE learners
                SET rfID = %s
                WHERE learner_id= %s""",
                 (newRFID,self.ID))

        return DB.query('SELECT rfid FROM learners WHERE learner_id= %s', (self.ID))[0]

