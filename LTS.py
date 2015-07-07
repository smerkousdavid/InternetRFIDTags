import sqlite3
import time

def wipearray():
    conn = sqlite3.connect('test.db')

    c = conn.cursor()
    c.execute("DROP TABLE IF EXISTS learners")

    c.execute('''CREATE TABLE learners
              (id INTEGER PRIMARY KEY AUTOINCREMENT ,
              fName TEXT,
              lName TEXT,
              year TEXT,
              notes TEXT,
              location INTEGER,
              checkInTime TEXT,
              rfID TEXT)''')

    conn.commit()
    conn.close()


### I did this because in the bottom class, creating a learner requires inserting fname and such else. In order to insert
### a name or whatever, you need an ID to insert it to. In order to get an ID, you need a name. Hence the issue
### Was there a better way of doing it? No doubt.
### Are there way more pressing issues? No doubt.
def createLearner(fName,lName,year=0):
    conn = sqlite3.connect('test.db')
    c = conn.cursor()

    c.execute("INSERT INTO learners (fName,lName,year,location,checkInTime) VALUES (:FirstName, :LastName, :year, 0, 0)",
              {"FirstName": fName, "LastName": lName, 'year': year})

    c.execute("""SELECT last_insert_rowid()""")

    id = c.fetchone()

    conn.commit()
    conn.close()

    return id[0]

def learnerfromRFID(RFID):
    """ Returns an LTS object from rfID """
    conn = sqlite3.connect('test.db')
    c = conn.cursor()
    try:
        c.execute("SELECT id FROM learners WHERE rfID = :rfID", {'rfID': RFID})
        id = c.fetchone()[0]
    except TypeError:
        raise ValueError('No learner with RFID:{}'.format(RFID))

    conn.commit()
    conn.close()

    return LTSBackend(id)

### WTF PYTHON? Why do I need to put this stupid object parameter in order for setters to work?
class LTSBackend(object):

    def __init__(self, ID):
        # Grr this is a weird way of doing it
        # Basically the "Nones" are just telling python to execute the properties
        self.ID = ID
        self._fName = None
        self._lName = None
        self._Location = None
        self._RFID = None
        self.Notes = None

    @property
    def fName(self):
        # TODO : Not be such a lazy bum and create something that will handle opening and closing of sqlite3 db
        conn = sqlite3.connect('test.db')
        c = conn.cursor()
        c.execute("SELECT fName FROM learners WHERE id = :ID", {'ID': self.ID})
        final = c.fetchone()

        conn.commit()
        conn.close()

        return final[0]

    @fName.setter
    def fName(self,newName):
        conn = sqlite3.connect('test.db')
        c = conn.cursor()

        c.execute("""UPDATE learners
                  SET fName = :newName
                  WHERE id= :id""",
                  {'id': self.ID, 'newName': newName})

        # The reason I do the select is to make sure that it actually inserted it into the database
        # I should probably remove these to save speed.
        c.execute("SELECT fName FROM learners WHERE id = :ID", {'ID': self.ID})
        final = c.fetchone()

        conn.commit()
        conn.close()

        self._fName = final[0]

    @property
    def lName(self):
        conn = sqlite3.connect('test.db')
        c = conn.cursor()

        c.execute("SELECT lName FROM learners WHERE id = :ID", {'ID': self.ID})
        final = c.fetchone()

        conn.commit()
        conn.close()

        return final[0]

    @lName.setter
    def lName(self,newName):
        conn = sqlite3.connect('test.db')
        c = conn.cursor()

        c.execute("""UPDATE learners
                  SET lName = :newName
                  WHERE id= :Identity""",
                  {'Identity': self.ID, 'newName': newName})
        c.execute("SELECT lName FROM learners WHERE id = :ID", {'ID': self.ID})
        final = c.fetchone()

        conn.commit()
        conn.close()

        self._lName = final[0]

    @property
    def Notes(self):
        conn = sqlite3.connect('test.db')
        c = conn.cursor()

        c.execute("SELECT notes FROM learners WHERE id = :ID", {'ID': self.ID})
        final = c.fetchone()

        conn.commit()
        conn.close()

        return final[0]

    @Notes.setter
    def Notes(self,newNotes):
        conn = sqlite3.connect('test.db')
        c = conn.cursor()

        c.execute("""UPDATE learners
                  SET notes = :newNotes
                  WHERE id= :Identity""",
                  {'Identity': self.ID, 'newNotes': newNotes})
        c.execute("SELECT notes FROM learners WHERE id = :ID", {'ID': self.ID})
        final = c.fetchone()

        conn.commit()
        conn.close()

        self._lName = final[0]

    @property
    def Location(self):
        conn = sqlite3.connect('test.db')
        c = conn.cursor()

        c.execute('SELECT location FROM learners WHERE id= :Identity',
                    {'Identity': self.ID})
        final = c.fetchone()

        conn.commit()
        conn.close()

        return final[0]

    @Location.setter
    def Location(self, location):
        conn = sqlite3.connect('test.db')
        c = conn.cursor()

        # This writes in the time that the check in happens

        epochTime = time.time()
        c.execute("""UPDATE learners
                  SET checkInTime = :newtime
                  WHERE id= :Identity""",
                  {'Identity': self.ID, 'newtime': epochTime})
        c.execute("""UPDATE learners
                  SET location = :newlocation
                  WHERE id= :Identity""",
                  {'Identity': self.ID, 'newlocation': location})
        c.execute("SELECT location FROM learners WHERE id = :ID", {'ID': self.ID})
        final = c.fetchone()

        conn.commit()
        conn.close()

        self._Location = final[0]

    @property
    def RFID(self):
        conn = sqlite3.connect('test.db')
        c = conn.cursor()

        c.execute('SELECT rfID FROM NFC WHERE id= :Identity',
                    {'Identity': self.ID})
        final = c.fetchone()

        conn.commit()
        conn.close()

        return final[0]

    @RFID.setter
    def RFID(self,newRFID):
        conn = sqlite3.connect('test.db')
        c = conn.cursor()

        c.execute("""UPDATE learners
              SET rfID = :RFID
              WHERE id= :Identity""",
          {'Identity': self.ID, 'RFID': newRFID})

        c.execute('SELECT rfID FROM learners WHERE id= :Identity',
            {'Identity': self.ID})
        final = c.fetchone()

        conn.commit()
        conn.close()
        self._RFID = final[0]