package np.com.sudanchapagain.call_history_viewer;

import com.formdev.flatlaf.FlatLightLaf;
import javax.swing.*;
import javax.swing.RowFilter;
import javax.swing.table.*;
import javax.xml.parsers.*;
import org.w3c.dom.*;
import java.awt.*;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.io.File;

public class Main extends JFrame {
    private final DefaultTableModel tableModel;
    private final TableRowSorter<DefaultTableModel> rowSorter;
    private final JTextField searchField;

    public Main() {
        super("call history viewer");

        try {
            UIManager.setLookAndFeel( new FlatLightLaf() );
        } catch (Exception e) {
            e.printStackTrace();
        }

        String[] columns = {"number", "duration", "readable date", "type", "contact name"};
        tableModel = new DefaultTableModel(columns, 0) {
            @Override
            public boolean isCellEditable(int row, int column) {
                return false;
            }
        };

        JTable table = new JTable(tableModel);
        table.setAutoCreateRowSorter(true);
        table.setFillsViewportHeight(true);
        table.setRowHeight(24);
        table.setFont(new Font("Inter", Font.PLAIN, 14));
        table.getTableHeader().setFont(new Font("Inter", Font.BOLD, 14));

        rowSorter = new TableRowSorter<>(tableModel);
        table.setRowSorter(rowSorter);

        JScrollPane scrollPane = new JScrollPane(table);

        JLabel searchLabel = new JLabel("search: ");
        searchField = new JTextField(30);
        searchField.setFont(new Font("Inter", Font.PLAIN, 14));
        searchField.addKeyListener(new KeyAdapter() {
            @Override
            public void keyReleased(KeyEvent e) {
                String text = searchField.getText();
                if (text.trim().isEmpty()) {
                    rowSorter.setRowFilter(null);
                } else {
                    rowSorter.setRowFilter(RowFilter.regexFilter("(?i)" + text));
                }
            }
        });

        JPanel searchPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        searchPanel.add(searchLabel);
        searchPanel.add(searchField);

        JButton openButton = new JButton("open file");
        openButton.setFont(new Font("Inter", Font.PLAIN, 14));
        openButton.addActionListener(e -> openFile());

        JPanel topPanel = new JPanel(new BorderLayout(5, 5));
        topPanel.add(openButton, BorderLayout.WEST);
        topPanel.add(searchPanel, BorderLayout.CENTER);
        topPanel.setBorder(BorderFactory.createEmptyBorder(8,8,8,8));

        add(topPanel, BorderLayout.NORTH);
        add(scrollPane, BorderLayout.CENTER);

        setSize(900, 600);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLocationRelativeTo(null);
    }

    private void openFile() {
        JFileChooser chooser = new JFileChooser();
        int res = chooser.showOpenDialog(this);
        if (res == JFileChooser.APPROVE_OPTION) {
            File xmlFile = chooser.getSelectedFile();
            loadCallsFromFile(xmlFile);
        }
    }

    private void loadCallsFromFile(File xmlFile) {
        try {
            tableModel.setRowCount(0);

            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            factory.setIgnoringComments(true);
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document doc = builder.parse(xmlFile);

            NodeList callNodes = doc.getElementsByTagName("call");

            for (int i = 0; i < callNodes.getLength(); i++) {
                Element call = (Element) callNodes.item(i);

                String number = call.getAttribute("number");
                String duration = call.getAttribute("duration");
                String readableDate = call.getAttribute("readable_date");
                String type = decodeCallType(call.getAttribute("type"));
                String contactName = call.getAttribute("contact_name");

                tableModel.addRow(new Object[]{number, duration, readableDate, type, contactName});
            }
        } catch (Exception e) {
            JOptionPane.showMessageDialog(this,
                    "Error loading XML: " + e.getMessage(),
                    "Error",
                    JOptionPane.ERROR_MESSAGE);
        }
    }

    private String decodeCallType(String type) {
        return switch (type) {
            case "1" -> "Incoming";
            case "2" -> "Outgoing";
            case "3" -> "Missed";
            default -> "Unknown";
        };
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> {
            new Main().setVisible(true);
        });
    }
}
