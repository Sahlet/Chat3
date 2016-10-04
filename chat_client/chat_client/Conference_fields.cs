using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Threading;

namespace chat_client
{
    delegate void Conference_fields_delegate(Conference_fields sender);
    public class Conference_fields
    {

        public static Dictionary<string, Conference_fields> Conferences;//id contact для конфере
        public static Conference_fields getConference_fields(string id){
            if (Conferences.ContainsKey(id)) return Conferences[id];
            else return null;
        }
        public static Conference_fields getConference_fields(string id, string name)
        {
            if (Conferences.ContainsKey(id)){
                Conferences[id].Name = name;
                return Conferences[id];
            }
            Conference_fields res = (Conferences[id] = new Conference_fields(id, name));
            Central.insert_conference(res);
            return res;
        }


        private bool unread;
        private string last_tick, name, id;
        public ItemList_with_Button_more messages, partners;
        public List<method_empty> UnreadChanged, Last_tickChanged;

        private Conference_fields(string id, string name)
        {
            UnreadChanged = new List<method_empty>();
            Last_tickChanged = new List<method_empty>();

            this.id = id;
            this.name = name;

            messages = new ItemList_with_Button_more(new method(this.load_more_messages), DockStyle.Fill, DockStyle.Top);
            messages.AutoScroll = true;
            partners = new ItemList_with_Button_more(new method(this.load_more_partners), DockStyle.Fill, DockStyle.Top);
            partners.AutoScroll = true;

            unread_and_last_tick_checker();
        }
        public Conference getConference()
        {
            return new Conference(this.ID, this.Name, Central.Chat.contextMenuStrip2CONFER);
        }

        public void unread_and_last_tick_checker()
        {
            int k = (int)Central.key.geter_conf_unread_last_tick;
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
        private void run_listeners(List<method_empty> methodsList){
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
        
        public bool Unread{
            get { return unread; }
            set
            {
                if (value != unread) {
                    unread = value;
                    run_listeners(UnreadChanged);
                }
            }
        }
        public string Last_tick{
            get { return last_tick; }
            set {
                if (value != last_tick) {
                    last_tick = value;
                    run_listeners(Last_tickChanged);
                }
            }
        }

        public string Name{
            get { return name; }
            private set {
                if (name != value) name = value;
            }
        }
        public string ID{
            get { return id; }
        }

        //для вызова из двугого потока с использованием инвок
        public void insert_message(object message_ID, object time, object message, object login)
        {
            insert_message((string)message_ID, (string)time, (string)message, (string)login);
        }
        public void insert_message(object m) { insert_message((Message)m); }
        //при использовании этой перегрузки метода не приходится создавать лишних слушателей событий
        public void insert_message(string message_ID, string time, string message, string login)
        {
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
        private void load_more_messages(object sender, EventArgs e)
        {
            load_more_messages();
        }
        public void load_more_partners(object sender, EventArgs e) {
        }
    }
}
