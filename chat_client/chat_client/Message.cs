using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace chat_client
{
    public partial class Message : UserControl
    {
        public string message_ID;
        private Control oldParent;
        private EventHandler Message_SizeChanged_Listener;
        public void init(string message_ID, string time, string message, User messager)
        {
            InitializeComponent();
            this.Size = new Size(40, 32);
            //this.panel1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
            //| System.Windows.Forms.AnchorStyles.Left)
            //| System.Windows.Forms.AnchorStyles.Right)));
            this.oldParent = null;
            this.Dock = DockStyle.Bottom;
            this.time.Text = time;
            this.messageText.Text = message;
            panel1.Controls.Add(messager);
            this.Name = message_ID;
            this.message_ID = message_ID;
            this.SizeChanged += (Message_SizeChanged_Listener = new EventHandler(this.Message_SizeChanged));
        }
        public Message(string message_ID, string time, string message, User messager)
        {
            init(message_ID, time, message, messager);
        }

        private void Parent_SizeChanged(object sender, EventArgs e)
        {
            this.MaximumSize = new Size(this.Parent.Size.Width, 0);
        }

        private void Message_ParentChanged(object sender, EventArgs e)
        {
            if (oldParent != null) oldParent.SizeChanged -= Parent_SizeChanged;
            oldParent = this.Parent;
            if (this.Parent != null) this.Parent.SizeChanged += Parent_SizeChanged;
            this.MaximumSize = new Size(this.Parent.Size.Width, 0);
        }

        private void Message_SizeChanged(object sender, EventArgs e)
        {
            this.SizeChanged -= Message_SizeChanged_Listener;
            this.messageText.MaximumSize = new Size(this.MaximumSize.Width, 0);
            //panel2.Size = new Size(this.Size.Width, this.messageText.Location.Y + this.messageText.Size.Height);
            panel1.Size = new Size(this.Size.Width, this.panel2.Location.Y + this.messageText.Location.Y + this.messageText.Size.Height);
            this.Size = new Size(this.Size.Width, panel1.Size.Height + 2);
            this.SizeChanged += Message_SizeChanged_Listener;
            messageText.MaximumSize = new Size(panel2.Size.Width, panel2.MaximumSize.Height);
        }
    }
}
