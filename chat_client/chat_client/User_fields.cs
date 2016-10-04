using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using System.Windows.Forms;

namespace chat_client
{
    delegate void User_fields_delegate(User_fields sender);
    delegate void for_setUserOnline(bool ch);
    public class User_fields
    {
        public static Dictionary<string, User_fields> Users;//name, user
        public static User_fields getUser_fields(string name){
            if (Users.ContainsKey(name)) return Users[name];
            int k = (int)Central.key.get_friend_id;
            if (Central.socks[k] == null) Central.set_right(k);
            if (Central.socks[k] == null) Central.Fatal_Error();
            Central.start_dialog(k);
            Central.write(name, Central.socks[k]);
            User_fields res = null;
            if (Central.read(Central.socks[k]) == Central.answers[0])
            {
                string id = Central.read(Central.socks[k]);
                if (id == "0") res = new User_fields(name);
                else res = new User_fields(id, name);
                Users[name] = res;
                Central.insert_friend(res);
            }
            else
            {
                MessageBox.Show(Central.read(Central.socks[k]), "Message from server.");
                Central.socks[k].Close();
                Central.set_right(k);
            }
            Central.stop_dialog(k);
            return res;
        }
        public static User_fields getUser_fields(string id, string name)
        {
            if (Users.ContainsKey(name)){
                Users[name].ID = id;
                if (id == null && Central.friends.Controls.ContainsKey(name)) Central.friends.Controls.RemoveByKey(name);
                else Central.insert_friend(Users[name]);
                return Users[name];
            }
            User_fields res = (Users[name] = new User_fields(id, name));
            Central.insert_friend(res);
            return res;
        }

        private bool unread, online;
        private string last_tick, name, id;
        public ItemList_with_Button_more messages;
        public List<method_empty> UnreadChanged, OnlineChanged, Last_tickChanged;

        private User_fields(string id, string name) : this(name)
        {
            ID = id;
        }
        private User_fields(string name)
        {
            UnreadChanged = new List<method_empty>();
            OnlineChanged = new List<method_empty>();
            Last_tickChanged = new List<method_empty>();
            this.name = name;

            int k = (int)Central.key.get_last_user_tick;
            if (Central.socks[k] == null) Central.set_right(k);
            if (Central.socks[k] == null) Central.Fatal_Error();
            DateTime now;
            Central.start_dialog(k);
            Central.write(Central.log, Central.socks[k]);
            Central.stop_dialog(k);
            if (Central.read(Central.socks[k]) == Central.answers[0])
            {
                now = DateTime.Parse(Central.read(Central.socks[k]));
                online_checker(now, Central.still_online);
            }
            else
            {
                MessageBox.Show(Central.read(Central.socks[k]), "Message from server.");
                Central.socks[k].Close();
                Central.set_right(k);
            }
        }
        public User getUser() {
            if (Central.friends.Controls.ContainsKey(this.Name)) return new User(this, Central.Chat.contextMenuStrip1FRIEND);
            else return new User(this, Central.Chat.contextMenuStrip3USERS);
        }

        public void unread_and_last_tick_checker()
        {
            if (this.ID == null || this.ID == "0") return;
            int k = (int)Central.key.geter_freand_unread_last_tick;
            if (Central.socks[k] == null) Central.set_right(k);
            if (Central.socks[k] == null) Central.Fatal_Error();
            Central.start_dialog(k);
            Central.write(this.ID, Central.socks[k]);
            if (Central.read(Central.socks[k]) == Central.answers[0])
            {
                this.Unread = Convert.ToBoolean(Convert.ToInt16(Central.read(Central.socks[k])));
                this.Last_tick = Central.read(Central.socks[k]);
            }
            else
            {
                MessageBox.Show(Central.read(Central.socks[k]), "Message from server.");
                Central.socks[k].Close();
                Central.set_right(k);
            }
            Central.stop_dialog(k);
        }
        public void online_checker(DateTime now, int still_online)
        {
            int k = (int)Central.key.get_last_user_tick;
            if (Central.socks[k] == null) Central.set_right(k);
            if (Central.socks[k] == null) Central.Fatal_Error();
            //for_setUserOnline set_userOnline = new for_setUserOnline(this.setUserOnline);
            Central.start_dialog(k);
            Central.write(this.name, Central.socks[k]);
            if (Central.read(Central.socks[k]) == Central.answers[0])
            {
                if (DateTime.Parse(Central.read(Central.socks[k])) > now.AddMinutes(still_online)) this.Online = false;
                else this.Online = true;
            }
            else
            {
                MessageBox.Show(Central.read(Central.socks[k]), "Message from server.");
                Central.socks[k].Close();
                Central.set_right(k);
            }
            Central.stop_dialog(k);
        }

        private void run_listeners(List<method_empty> methodsList)
        {
            int n = methodsList.Count;
            for (int i = 0; i < n; i++)
            {
                try { methodsList[i](); }
                catch (Exception e)
                {
                    MessageBox.Show(e.ToString(), "Error.");
                }
            }
        }

        public bool Unread
        {
            get { return unread; }
            set
            {
                if (value != unread)
                {
                    unread = value;
                    run_listeners(UnreadChanged);
                }
            }
        }
        public bool Online
        {
            get { return online; }
            set
            {
                if (value != online)
                {
                    online = value;
                    run_listeners(OnlineChanged);
                }
            }
        }
        public string Last_tick
        {
            get { return last_tick; }
            set
            {
                if (value != last_tick)
                {
                    last_tick = value;
                    run_listeners(Last_tickChanged);
                }
            }
        }

        public string Name
        {
            get { return name; }
        }
        public string ID
        {
            get { return id; }
            private set {
                if (id != value)
                {
                    id = value;
                    if (value != null)
                    {
                        messages = new ItemList_with_Button_more(new method(this.load_more_messages), DockStyle.Fill, DockStyle.Top);
                        messages.AutoScroll = true;
                        unread_and_last_tick_checker();
                    } else messages = null;
                }
            }
        }

        //для вызова из двугого потока с использованием инвок
        public void insert_message(object message_ID, object time, object message, object login) {
            insert_message((string)message_ID, (string)time, (string)message, (string)login);
        }
        public void insert_message(object m) { insert_message((Message)m); }
        //при использовании этой перегрузки метода не приходится создавать лишних слушателей событий
        public void insert_message(string message_ID, string time, string message, string login) {
            if (!messages.Controls.ContainsKey(message_ID)) insert_message(new Message((string)message_ID, (string)time, (string)message, User.getUser((string)login)));
        }
        public void insert_message(Message m)
        {
            if (!messages.Controls.ContainsKey(m.Name))
            {
                messages.Controls.Add(m);
                if (messages.Controls.Count > 2)
                {
                    DateTime time_of_message = new DateTime();
                    time_of_message = DateTime.Parse(m.time.Text);
                    foreach (object obj_ in messages.Controls)
                    {
                        try
                        {
                            Message obj = (Message)obj_;
                            if (DateTime.Parse(obj.time.Text) >= time_of_message)
                            {
                                messages.Controls.SetChildIndex(m, messages.Controls.IndexOf(obj));
                                if (messages.Controls.IndexOf(m) == messages.Controls.Count - 1) messages.VerticalScroll.Value = messages.VerticalScroll.Maximum;
                                break;
                            }
                        }
                        catch (Exception) { }
                    }
                }
            }
        }
        private void load_more_messages(object sender, EventArgs e){
            load_more_messages();
        }
        public void load_more_messages()
        {
            int k = (int)Central.key.geter_n_m_messages_p;

            if (Central.socks[k] == null) Central.set_right(k);
            if (Central.socks[k] == null) Central.Fatal_Error();
            Central.start_dialog(k);
            Central.write((messages.Controls.Count - 1).ToString(), Central.socks[k]);
            Central.write((messages.Controls.Count - 1 + 10).ToString(), Central.socks[k]);
            Central.write(ID, Central.socks[k]);
            if (Central.read(Central.socks[k]) == Central.answers[0])
            {
                int n = Convert.ToInt32(Central.read(Central.socks[k]));
                Message[] messages_ = new Message[n];
                User messager;
                string message_ID, message, login, time;
                for (int i = 0; i < n; i++)
                {
                    message_ID = Central.read(Central.socks[k]);
                    message = Central.read(Central.socks[k]);
                    login = Central.read(Central.socks[k]);
                    time = Central.read(Central.socks[k]);
                    messager = User.getUser(login);
                    this.insert_message(new Message(message_ID, time, message, messager));
                }
            }
            else
            {
                MessageBox.Show(Central.read(Central.socks[k]), "Message from server.");
                Central.socks[k].Close();
                Central.set_right(k);
            }
            Central.stop_dialog(k);
        }
    }
}
