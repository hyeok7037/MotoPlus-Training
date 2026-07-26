using System.Net.Sockets;
using System.Text;

namespace RobotStringClient;

public sealed class MainForm : Form
{
    private const int MaxAsciiBytes = 32;
    private readonly TextBox ipTextBox = new() { Text = "192.168.255.1", Width = 140 };
    private readonly NumericUpDown portBox = new() { Minimum = 1, Maximum = 65535, Value = 11000, Width = 90 };
    private readonly TextBox messageTextBox = new() { Width = 300, MaxLength = MaxAsciiBytes };
    private readonly Label lengthLabel = new() { AutoSize = true, Text = "0 / 32 bytes" };
    private readonly Button sendButton = new() { Text = "Send to S010", AutoSize = true };
    private readonly TextBox logTextBox = new() { Multiline = true, ReadOnly = true, ScrollBars = ScrollBars.Vertical, Dock = DockStyle.Fill };

    public MainForm()
    {
        Text = "MotoPlus S010 TCP Client";
        Width = 610; Height = 360; StartPosition = FormStartPosition.CenterScreen;
        BuildLayout();
        messageTextBox.KeyPress += MessageTextBox_KeyPress;
        messageTextBox.TextChanged += MessageTextBox_TextChanged;
        sendButton.Click += SendButton_Click;
    }

    private void BuildLayout()
    {
        var top = new FlowLayoutPanel { Dock = DockStyle.Top, AutoSize = true, Padding = new Padding(10) };
        top.Controls.Add(new Label { Text = "Robot IP:", AutoSize = true, Margin = new Padding(0,7,4,0) });
        top.Controls.Add(ipTextBox);
        top.Controls.Add(new Label { Text = "Port:", AutoSize = true, Margin = new Padding(12,7,4,0) });
        top.Controls.Add(portBox);

        var msg = new FlowLayoutPanel { Dock = DockStyle.Top, AutoSize = true, Padding = new Padding(10,0,10,10) };
        msg.Controls.Add(new Label { Text = "ASCII:", AutoSize = true, Margin = new Padding(0,7,4,0) });
        msg.Controls.Add(messageTextBox); msg.Controls.Add(lengthLabel); msg.Controls.Add(sendButton);

        var logPanel = new Panel { Dock = DockStyle.Fill, Padding = new Padding(10) };
        logPanel.Controls.Add(logTextBox);
        Controls.Add(logPanel); Controls.Add(msg); Controls.Add(top);
    }

    private void MessageTextBox_KeyPress(object? sender, KeyPressEventArgs e)
    {
        if(char.IsControl(e.KeyChar)) return;
        if(e.KeyChar < 0x20 || e.KeyChar > 0x7E) { e.Handled = true; System.Media.SystemSounds.Beep.Play(); }
    }

    private void MessageTextBox_TextChanged(object? sender, EventArgs e)
    {
        string ascii = KeepPrintableAscii(messageTextBox.Text);
        if(ascii.Length > MaxAsciiBytes) ascii = ascii[..MaxAsciiBytes];
        if(ascii != messageTextBox.Text) { messageTextBox.Text = ascii; messageTextBox.SelectionStart = ascii.Length; }
        lengthLabel.Text = $"{Encoding.ASCII.GetByteCount(ascii)} / 32 bytes";
    }

    private async void SendButton_Click(object? sender, EventArgs e)
    {
        string message = KeepPrintableAscii(messageTextBox.Text);
        if(string.IsNullOrEmpty(message)) { MessageBox.Show("Enter an ASCII string."); return; }
        sendButton.Enabled = false;
        try
        {
            using var client = new TcpClient();
            using var timeout = new CancellationTokenSource(TimeSpan.FromSeconds(5));
            await client.ConnectAsync(ipTextBox.Text.Trim(), (int)portBox.Value, timeout.Token);
            await using NetworkStream stream = client.GetStream();
            byte[] sendData = Encoding.ASCII.GetBytes(message + "\n");
            await stream.WriteAsync(sendData, timeout.Token);
            await stream.FlushAsync(timeout.Token);
            string response = await ReadLineAsync(stream, timeout.Token);
            AddLog($"TX: {message}"); AddLog($"RX: {response}");
        }
        catch(Exception ex) { AddLog($"ERROR: {ex.Message}"); }
        finally { sendButton.Enabled = true; }
    }

    private static string KeepPrintableAscii(string text)
    {
        var b = new StringBuilder();
        foreach(char ch in text) if(ch >= 0x20 && ch <= 0x7E) b.Append(ch);
        return b.ToString();
    }

    private static async Task<string> ReadLineAsync(NetworkStream stream, CancellationToken token)
    {
        var result = new List<byte>(); var buffer = new byte[1];
        while(result.Count < 128)
        {
            int count = await stream.ReadAsync(buffer.AsMemory(0,1), token);
            if(count == 0 || buffer[0] == (byte)'\n') break;
            if(buffer[0] != (byte)'\r') result.Add(buffer[0]);
        }
        return Encoding.ASCII.GetString(result.ToArray());
    }

    private void AddLog(string message) => logTextBox.AppendText($"[{DateTime.Now:HH:mm:ss}] {message}{Environment.NewLine}");
}
