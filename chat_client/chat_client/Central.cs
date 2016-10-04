using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Threading;
using System.Net.Sockets;
using System.Windows.Forms;

namespace chat_client
{
    public static class Central
    {
        public static method delegate_load_more_to = new method(Central.load_more_to);
        [STAThread]
        public static void Main() {
            Init();
            Chat.ShowDialog();
        }
        //чтоб проверять впервые ли запущ прогр.
        static private bool first = true;
        public static void Init() {
            if (first)
            {
                Application.EnableVisualStyles();
                Application.SetCompatibleTextRenderingDefault(false);
                //Application.Run(new Form1());
                first = false;
            }
            Central.closing = false;
            Central.counter = new UInt64[Central.n_keys, 2];
            Central.socks = new Socket[Central.n_keys];
            Central.threads = new List<Thread>();
            Form2 form = new Form2("User params.", "Login", "Password", Central.commands, Central.answers, Central.keys[0]);
            form.ShowDialog();
            if (!form.aut) Environment.Exit(0);

            friends = new ItemList_with_Button_more(delegate_load_more_to, DockStyle.Fill, DockStyle.Bottom);
            friends.Tag = (object)Central.key.geter_n_m_friens;
            conferences = new ItemList_with_Button_more(delegate_load_more_to, DockStyle.Fill, DockStyle.Bottom);
            conferences.Tag = (object)Central.key.geter_n_m_confs;
            searching_panel = new ItemList_with_Button_more(delegate_load_more_to, DockStyle.Fill, DockStyle.Bottom);
            searching_panel.Tag = (object)Central.key.geter_n_m_users;
            requests = new ItemList_with_Button_more(delegate_load_more_to, DockStyle.Fill, DockStyle.Bottom);
            requests.Tag = (object)Central.key.geter_n_m_requests;

            Conference_fields.Conferences = new Dictionary<string, Conference_fields>();
            User_fields.Users = new Dictionary<string, User_fields>();

            Central.log = form.getParam1();
            Central.pass = form.getParam2();

            Chat = new Form1();
            refresh_data();

            Thread online_ = new System.Threading.Thread(new System.Threading.ThreadStart(Central.online));
            Central.threads.Add(online_);
            Thread online_checker_ = new System.Threading.Thread(new System.Threading.ThreadStart(Central.online_checker));
            Central.threads.Add(online_checker_);
            Thread message_loader_ = new System.Threading.Thread(new System.Threading.ThreadStart(Central.message_loader));
            Central.threads.Add(message_loader_);
            online_.Start();
            online_checker_.Start();
            message_loader_.Start();
        }
        public static void Close(){
            if (Central.socks != null)
            {
                Central.closing = true;
                for (int i = 0; i < Central.socks.Length; i++)
                {
                    if (Central.socks[i] != null) Central.socks[i].Close();
                    Central.socks[i] = null;
                }
            }
            if (Central.threads != null)
            {
                foreach (Thread t in Central.threads) { t.Abort(); }
                Central.threads.Clear();
            }
            Central.threads = null;
            Central.socks = null;
            friends.Clear();
            conferences.Clear();
            searching_panel.Clear();
            requests.Clear();
            User_fields.Users.Clear();
            Conference_fields.Conferences.Clear();
        }

        //public static delegate string[] int_parsms_string(int k, int n_read, params string[] s);
        //public static int_parsms_string for_write_and_read = new int_parsms_string(write_and_read);
        ////пишет и читает данны по к-тому сокету в порядке очереди запросов;
        ////n_read - еоличество строк, что надо прочесть
        //public static string[] write_and_read(int k, int n_read, params string[] s) {
        //    while (counter[k, 0] != counter[k, 1]) { System.Threading.Thread.Sleep(5); }
        //    if (socks[k] == null) set_right(k);
        //    ++counter[k, 0];
        //    for (int i = 0; i < s.Length; i++) write(s[i], socks[k]);
        //    string[] res = new string[n_read];
        //    for (int i = 0; i < n_read; i++) res[i] = read(socks[k]);
        //    ++counter[k, 1];
        //    return res;
        //}

        public static void start_dialog(int k)
        {
            while (counter[k, 0] != counter[k, 1]) { System.Threading.Thread.Sleep(5); }
            ++counter[k, 0];
        }
        public static void stop_dialog(int k) { ++counter[k, 1]; }
        
        public const int n_commands = 2, n_answers = 2, n_keys = 21;
        static public string[] commands = { "aut", "reg" }, answers = { "0", "1" }, keys = { "check", "smp", "smc", "sr", "ar", "cc", "ap", "line", "sa", "gud", "glut", "gnmp", "gnmr", "gnmmp", "gnmc", "gnmf", "gnmco", "gnmu", "gful", "gcul", "gfid"};//массив ключей команды авторизации
        public enum key { check, sender_private_message, sender_chat_message, sender_request, accepter_request, creater_chat, adder_part, line, seter_access, get_user_data, get_last_user_tick, geter_n_m_partners_from, geter_n_m_requests, geter_n_m_messages_p, geter_n_m_messages_c, geter_n_m_friens, geter_n_m_confs, geter_n_m_users, geter_freand_unread_last_tick, geter_conf_unread_last_tick, get_friend_id };
        public const ushort max_message_len = 255 * 257;

        
        static public string log, pass;


        public static ItemList_with_Button_more friends, conferences, searching_panel, requests;
        public static void load_more_to(object sender, EventArgs e){
            ItemList_with_Button_more list = (ItemList_with_Button_more)sender;
            int k = (int)list.Tag;
            if (socks[k] == null) set_right(k);
            if (socks[k] == null) Fatal_Error();
            start_dialog(k);
            write("0", Central.socks[k]);
            write((list.get_n_visible_children() - 1 + 10).ToString(), Central.socks[k]);
            write(Chat.textBox1.Text, Central.socks[k]);
            //if (list == conferences || list == friends) Central.write("0", Central.socks[k]);
            if (Central.read(Central.socks[k]) == Central.answers[0])
            {
                int n = Convert.ToInt32(Central.read(Central.socks[k]));
                if (list == friends) for (int i = 0; i < n; i++) User_fields.getUser_fields(Central.read(Central.socks[k]), Central.read(Central.socks[k]));
                else if (list == conferences)
                    for (int i = 0; i < n; i++)
                    {
                    }
                else {
                    User[] variable = new User[n];
                    if (list == searching_panel) for (int i = n - 1; i >= 0; i--) variable[i] = User_fields.getUser_fields(null, Central.read(Central.socks[k])).getUser();
                    else if (list == requests){
                        for (int i = n - 1; i >= 0; i--){
                            variable[i] = User_fields.getUser_fields(null, Central.read(Central.socks[k])).getUser();
                            variable[i].Tag = Central.read(Central.socks[k]);
                        }
                    }
                    list.Clear();
                    list.Controls.AddRange(variable);
                }
            }
            else{
                MessageBox.Show(Central.read(Central.socks[k]), "Message from server.");
                Central.socks[k].Close();
                Central.set_right(k);
            }
            stop_dialog(k);
        }
        public static void refresh_data()
        {
            //this.Controls.SetChildIndex - испозьзовать!
            Central.no_selected();
            load_accaunt_data();
            Central.load_more_to(Central.friends, null);
            Central.load_more_to(Central.conferences, null);
        }
        public static string get_user_status(string name)
        {
            int k = (int)Central.key.get_user_data;
            Waiter w = new Waiter(ref Central.counter[k, 0], ref Central.counter[k, 1], new method_obj1(Central.get_user_status_), name);
            return (string)w.res;
        }
        public static object get_user_status_(object name)
        {
            int k = (int)Central.key.get_user_data;
            if (Central.socks[k] == null) Central.socks[k] = Central.make_authorised_sock(k);
            if (Central.socks[k] == null) return null;
            Central.write((string)name, Central.socks[k]);
            return Central.read(Central.socks[k]);
        }
        //для развития программы (авки загрузить,если сделаю)
        public static void load_accaunt_data() { }

        public static void no_selected()
        {
            Central.selected_num = 0;
            Central.selected_obj = null;
            Chat.panel6.Controls.Clear();
            Chat.panel7.Enabled = false;
        }
        public static void ShowMessages(User_fields user)
        {
            if (user.ID != null){
                bool exec = false;
                if (selected_num != 1) exec = true;
                else if(user.Name != ((User_fields)selected_obj).Name) exec = true;
                if (exec) {
                    selected_obj = user;
                    selected_num = 1;
                    Chat.panel6.Controls.Clear();
                    Chat.panel6.Controls.Add(user.messages);
                    Chat.panel6.Controls.Add(user.getUser());
                    if (user.messages.Controls.Count <= 1) { user.load_more_messages(); }//сделать!
                    Chat.panel7.Enabled = true;
                }

            }
            else no_selected();
        }
        public static void ShowMessages(Conference_fields conf)
        {
            if (conf.ID != null){
                bool exec = false;
                if (selected_num != 2) exec = true;
                else if(conf.ID != ((Conference_fields)selected_obj).ID) exec = true;
                if (exec) {
                    selected_obj = conf;
                    selected_num = 2;
                    Chat.panel6.Controls.Clear();
                    Chat.panel6.Controls.Add(conf.messages);
                    Chat.panel6.Controls.Add(conf.getConference());
                    if (conf.messages.Controls.Count <= 1) { conf.load_more_messages(); }//сделать!
                    Chat.panel7.Enabled = true;
                }
            }
            else no_selected();
        }

        public static void insert_friend(User_fields fields) {
            if (fields.ID != null)
            {
                if (!friends.Controls.ContainsKey(fields.Name))
                {
                    User u = fields.getUser();
                    friends.Controls.Add(u);
                    u.Last_tickChanged();//чтоб поставить на нужное место
                    u.ContextMenuStrip = Chat.contextMenuStrip1FRIEND;
                    requests.Controls.RemoveByKey(fields.Name);
                    searching_panel.Controls.RemoveByKey(fields.Name);
                }
            }
        }
        public static void insert_conference(Conference_fields fields) {
            if (!conferences.Controls.ContainsKey(fields.Name))
            {
                Conference c = fields.getConference();
                conferences.Controls.Add(c);
                c.Last_tickChanged();//чтоб поставить на нужное место
            }
        }

        static public Form1 Chat;
        static public Socket[] socks;
        public static List<System.Threading.Thread> threads = new List<System.Threading.Thread>();
        public static Thread send_mess_in_new_thread;
        //для выполнения взятия результата в нужном порядке
        static public UInt64[,] counter;

        
        static public bool closing;//хранит значение true, если сейчас должен произойти разрыв связи с сервером.(чтоб эксепшн не вылетал)

        
        //востанавливает сокет под номером k
        public static void set_right(int k){
            socks[k] = make_authorised_sock(k);
        }
        public static Socket make_authorised_sock(int k)
        {
            Socket sock = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            connect(sock);
            write(commands[0], sock);
            write(log, sock);
            write(pass, sock);
            write(keys[(int)k], sock);
            if (read(sock) != answers[0])
            {
                MessageBox.Show(read(sock), "Message from server.");
                return null;
            }
            return sock;
        }

        public static int still_online = 2;//количество минут
        public static void online_checker(){
            Thread.Sleep(200);
            int k = (int)Central.key.get_last_user_tick;
            DateTime now = new DateTime();
            while(true){
                Central.start_dialog(k);
                if (Central.socks[k] == null) Central.set_right(k);
                if (Central.socks[k] == null) Central.Fatal_Error();
                Central.write(Central.log, Central.socks[k]);
                if (Central.read(Central.socks[k]) == Central.answers[0]) now = DateTime.Parse(Central.read(Central.socks[k]));
                else{
                    MessageBox.Show(Central.read(Central.socks[k]), "Message from server.");
                    Central.socks[k].Close();
                    Central.set_right(k);
                    Central.stop_dialog(k);
                    Thread.Sleep(100000);//100 sec
                    continue;
                }
                Central.stop_dialog(k);
                var ref_ = User_fields.Users.Values.GetEnumerator();
                ref_.MoveNext();
                int count = User_fields.Users.Values.Count;
                try { for (int i = 0; i < count; i++, ref_.MoveNext()) { ref_.Current.online_checker(now, still_online); } }
                catch (Exception) {}
                Thread.Sleep(100000);//100 sec
            }
        }

        //отсылает сообщение текущему собеседнику
        public static void send_mess(string mess)
        {
            int k = 0;
            string ID = "";
            if (mess == "" || selected_obj != null && (selected_num == 1 || selected_num == 2))
            {
                if (Central.selected_num == 1)
                {
                    k = (int)Central.key.sender_private_message;
                    ID = ((User_fields)selected_obj).ID;
                }
                else if (Central.selected_num == 2)
                {
                    k = (int)Central.key.sender_chat_message;
                    ID = ((Conference_fields)selected_obj).ID;
                }
            }
            else { return; }

            if (Central.socks[k] == null) Central.set_right(k);
            if (Central.socks[k] == null) Central.Fatal_Error();
            Central.start_dialog(k);
            Central.write(ID, Central.socks[k]);
            Central.write(mess, Central.socks[k]);
            if (Central.read(Central.socks[k]) != Central.answers[0])
            {
                MessageBox.Show(Central.read(Central.socks[k]), "Message from server.");
                Central.socks[k].Close();
                Central.set_right(k);
            }
            Central.stop_dialog(k);
        }
        public static void send_mess(object mess)
        {
            send_mess((string)mess);
        }

        public static void there_is_unreed_user()
        {
            var ref_ = User_fields.Users.Values.GetEnumerator();
            ref_.MoveNext();
            int count = User_fields.Users.Values.Count;
            try { for (int i = 0; i < count; i++, ref_.MoveNext()) { ref_.Current.unread_and_last_tick_checker(); } }
            catch (Exception) { }
        }
        public static void there_is_anreed_conference()
        {
            var ref_ = Conference_fields.Conferences.Values.GetEnumerator();
            ref_.MoveNext();
            int count = Conference_fields.Conferences.Values.Count;
            try { for (int i = 0; i < count; i++, ref_.MoveNext()) { ref_.Current.unread_and_last_tick_checker(); } }
            catch (Exception) { }
        }
        public static void there_is_anreed_request()
        {
            load_more_to(requests, null);
        }

        public static void online()
        {
            Thread.Sleep(200);
            int k = (int)Central.key.line;
            if (Central.socks[k] == null) Central.set_right(k);
            if (Central.socks[k] == null) Central.Fatal_Error();
            while (true)
            {
                Central.write("", Central.socks[k]);
                if (Central.read(Central.socks[k]) == Central.answers[0])
                {
                    if (Central.read(Central.socks[k]) == "1") Chat.panel6.BeginInvoke(new method_empty(there_is_unreed_user));
                    if (Central.read(Central.socks[k]) == "1") Chat.panel6.BeginInvoke(new method_empty(there_is_anreed_conference));
                    if (Central.read(Central.socks[k]) == "1") Chat.panel6.BeginInvoke(new method_empty(there_is_anreed_request));
                }
                else
                {
                    MessageBox.Show(Central.read(Central.socks[k]), "Message from server.");
                    Central.socks[k].Close();
                    Central.set_right(k);
                }
                Thread.Sleep(500);
            }
        }

        delegate void method_one_obj_param(object obj);
        delegate void method_two_obj_param(object obj1, object obj2);
        delegate void method_four_obj_param(object obj1, object obj2, object obj3, object obj4);
        public static void User_fields_set_Unread(object fields, object bool_var)
        {
            ((User_fields)fields).Unread = (bool)bool_var;
        }
        public static void Conference_fields_set_Unread(object fields, object bool_var)
        {
            ((Conference_fields)fields).Unread = (bool)bool_var;
        }
        public static void message_loader() {
            int k = (int)Central.key.geter_n_m_messages_p;
            if (Central.socks[k] == null) Central.set_right(k);
            if (Central.socks[k] == null) Central.Fatal_Error();
            k = (int)Central.key.geter_n_m_messages_c;
            if (Central.socks[k] == null) Central.set_right(k);
            if (Central.socks[k] == null) Central.Fatal_Error();
            string ID = null;
            int n_messages = 2; //количество сообщени которые можно читать одним блоком
            ItemList_with_Button_more messages = null;
            object selected = null;
            method_two_obj_param User_fields_set_Unread_delegate = new method_two_obj_param(User_fields_set_Unread);
            method_two_obj_param Conference_fields_set_Unread_delegate = new method_two_obj_param(Conference_fields_set_Unread);

            while (true)
            {
                Thread.Sleep(100);
                if (selected_num == 0 || selected_obj == null) continue;
                selected = selected_obj;
                if (selected_num == 1){
                    k = (int)Central.key.geter_n_m_messages_p;
                    messages = ((User_fields)selected).messages;
                    ID = ((User_fields)selected).ID;
                }
                else if (selected_num == 2){
                    k = (int)Central.key.geter_n_m_messages_c;
                    messages = ((Conference_fields)selected).messages;
                    ID = ((Conference_fields)selected).ID;
                }
                Central.start_dialog(k);
                bool enought = false;
                int numb = 0;
                while (!enought){
                    Central.write((numb).ToString(), Central.socks[k]);
                    Central.write((numb = numb + n_messages).ToString(), Central.socks[k]);
                    Central.write(ID, Central.socks[k]);
                    if (Central.read(Central.socks[k]) == Central.answers[0])
                    {
                        int n = Convert.ToInt32(Central.read(Central.socks[k]));
                        Message[] messages_ = new Message[n];
                        string message_ID, message, login, time;
                        for (int i = 0; i < n; i++)
                        {
                            message_ID = Central.read(Central.socks[k]);
                            message = Central.read(Central.socks[k]);
                            login = Central.read(Central.socks[k]);
                            time = Central.read(Central.socks[k]);
                            if (!messages.Controls.ContainsKey(message_ID)){
                                if (k == (int)Central.key.geter_n_m_messages_p) Chat.BeginInvoke(new method_four_obj_param(((User_fields)selected).insert_message), message_ID, time, message, login);
                                else if (k == (int)Central.key.geter_n_m_messages_c) Chat.BeginInvoke(new method_four_obj_param(((Conference_fields)selected).insert_message), message_ID, time, message, login);
                            }
                            else {
                                i++;
                                for (; i < n; i++){
                                    Central.read(Central.socks[k]);
                                    Central.read(Central.socks[k]);
                                    Central.read(Central.socks[k]);
                                    Central.read(Central.socks[k]);
                                }
                                enought = true;
                                break;
                            }
                        }
                        if (n < n_messages) enought = true;
                    }
                    else
                    {
                        MessageBox.Show(Central.read(Central.socks[k]), "Message from server.");
                        Central.socks[k].Close();
                        Central.set_right(k);
                    }
                }
                Central.stop_dialog(k);
                if (k == (int)Central.key.geter_n_m_messages_p) Chat.BeginInvoke(User_fields_set_Unread_delegate, selected, false);
                else if (k == (int)Central.key.geter_n_m_messages_c) Chat.BeginInvoke(Conference_fields_set_Unread_delegate, selected, false);
            }
        }
        
        public static object selected_obj;
        public static int selected_num = 0; //0 - nothing, 1 - User_fields, 2 - Conference_fields

        public static void Fatal_Error(string s)
        {
            MessageBox.Show(s, "ERROR.");
            Environment.Exit(1);
        }
        public static void Fatal_Error()
        {
            Fatal_Error("Программа не может работать.");
        }


        public static void write(string s, Socket sock)
        {
            try
            {
                write(Encoding.UTF8.GetBytes(s), sock);
            }
            catch (Exception ex) {
                if (!closing)
                {
                    MessageBox.Show(ex.ToString(), "ERROR.");
                    Environment.Exit(4);
                }
            }
        }
        public static void write(byte[] buf, Socket sock)
        {
            if (buf.Length > max_message_len) throw new Exception("Could not send so long message.\nYour message weighs " + buf.Length.ToString() + " bytes, which is greater than the maximum value " + max_message_len.ToString() + ".");
            byte[] len = { (byte)(buf.Length / 256), (byte)(buf.Length % 256) };
            try
            {
                sock.Send(len, 2, SocketFlags.None);
                if (buf.Length != 0) sock.Send(buf, buf.Length, SocketFlags.None);
            }
            catch (Exception) {
                if (!closing) throw;
            }
        }
        public static string read(Socket sock)
        {
            try
            {
                return Encoding.UTF8.GetString(readBytes(sock));
            }
            catch (Exception ex) {
                if (!closing) {
                    MessageBox.Show(ex.ToString(), "ERROR.");
                    Environment.Exit(5);
                }
                return null;
            }
        }
        public static byte[] readBytes(Socket sock)
        {
            byte[] buf = new byte[2];
            try
            {
                sock.Receive(buf, 2, SocketFlags.None);
                buf = new byte[buf[0] * 256 + buf[1]];
                if (buf.Length != 0) sock.Receive(buf, buf.Length, SocketFlags.None);
                return buf;
            }
            catch (Exception) {
                throw;
            }
        }
        ////////////////////////////////////////////////////////////////
        //для ip, port
        const string IPEnvironmentVariable = "IP_of_chat_server";
        const string PortEnvironmentVariable = "Port_of_chat_server";
        public static void connect(Socket sock)
        {
            string ip = Environment.GetEnvironmentVariable(IPEnvironmentVariable);
            if (ip == null)
            {
                MessageBox.Show("Could not get environment variable \"" + IPEnvironmentVariable + "\".", "ERROR.");
                Environment.Exit(1);
            }
            string port = Environment.GetEnvironmentVariable(PortEnvironmentVariable);
            if (port == null)
            {
                MessageBox.Show("Could not get environment variable \"" + PortEnvironmentVariable + "\".", "ERROR.");
                Environment.Exit(2);
            }
            try
            {
                sock.Connect(ip, Convert.ToUInt16(port));
            }
            catch (Exception e)
            {
                MessageBox.Show(e.ToString(), "ERROR.");
                Environment.Exit(3);
            }
        }
    }
}
