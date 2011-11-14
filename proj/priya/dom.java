import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.Element;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.util.HashMap;
 
public class dom {
 
 public static void main(String argv[]) {
 
 try {
 
    HashMap<String, Integer> var = new HashMap<String, Integer>();
    HashMap varDef = new HashMap();
    HashMap varTab = new HashMap();
    String networkStr="";
    
    if(argv.length>0)
    {
    	networkStr=argv[0];
    	System.out.println("//Network -"+argv[0]);
    }
    else
    {
    	networkStr="network";
    }
    BufferedWriter fout=new BufferedWriter(new FileWriter("example_"+networkStr+".cpp"));
    String cppstart="#include <dai/factorgraph.h>\n#include <iostream>\n#include <fstream>\nusing namespace std;\nusing namespace dai;\nint main() {\n";
    System.out.println(cppstart);
    fout.write(cppstart);    
    File fXmlFile = new File(networkStr+".xml");
    
    DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
    DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
    Document doc = dBuilder.parse(fXmlFile);
    doc.getDocumentElement().normalize();
 
    //System.out.println("Root element :" + doc.getDocumentElement().getNodeName());
    NodeList nList = doc.getElementsByTagName("VARIABLE");
    System.out.println("-----------------------" + nList.getLength());
 
    for (int temp = 0; temp < nList.getLength(); temp++) {
 
       Node nNode = nList.item(temp);	    
       if (nNode.getNodeType() == Node.ELEMENT_NODE) {
 
          Element eElement = (Element) nNode;
          String name = getNameValue("NAME",eElement);
          String[] outcome = getTagValue("OUTCOME",eElement);
          var.put(name, outcome.length);
          System.out.println("Var " + name+"("+temp+","+ outcome.length+");" );
          //System.out.println("Number of values : "  + outcome.length);
          fout.write("Var " + name+"("+temp+","+ outcome.length+");\n");
      }
    }
    
    NodeList nListDef = doc.getElementsByTagName("PROBABILITY");  // or PROBABILITY
    String[] factorLineArr= new String[nListDef.getLength()];
    int givenlen=-1;
    for (int temp = 0; temp < nListDef.getLength(); temp++) {
    	 
        Node nNodeDef = nListDef.item(temp);	    
        if (nNodeDef.getNodeType() == Node.ELEMENT_NODE) {
  
           Element eElement = (Element) nNodeDef;
           String forName = getNameValue("FOR",eElement);
           String[] given = getTagValue("GIVEN",eElement);
           String[] table = getTagValue("TABLE",eElement);
           varDef.put(forName, given);
           varTab.put(forName, table);
          
           givenlen=given.length;
           String factor="Factor ";
           String factorLine = "P_"+forName;
           
           if (givenlen==0)
           {
        	   System.out.println( factor+"P_"+forName+"(  "+forName+"  );");
        	   fout.write(factor+"P_"+forName+"(  "+forName+"  );\n");
        	   factorLineArr[temp]="P_"+forName;
           }
           else
           {
        	   factorLine=factorLine+"_given";
        	   String strVector="a"+temp;
        	   String varset="( VarSet( "+strVector+".begin(), "+strVector+".end() )";
        	   
        	   System.out.println("std::vector<Var> "+strVector+";");
        	   fout.write("std::vector<Var> "+strVector+";\n");
        	   
        	   for(int i=0; i<givenlen; i++)
               {
        		   factorLine=factorLine+"_"+given[i];
        		   System.out.println(strVector+".push_back( "+given[i]+" );");
        		   fout.write(strVector+".push_back( "+given[i]+" );\n");
        		   //varset=varset+","+given[i];
               }
        	   factorLineArr[temp]=factorLine;
               System.out.println(factor+factorLine+varset+"  | "+forName+" );");
               fout.write(factor+factorLine+varset+"  | "+forName+" );\n");
               givenlen=-1;
           }
           
               
           //int tablelen=table.length;
           String tableLine = factorLine+".set(";
           String[] tableVal=table[0].split("\\s");
           
           for(int i=0; i<tableVal.length; i++)
           {
        	   String tableL =tableLine+i+", "+tableVal[i]+")";
        	   System.out.println(tableL +";");
        	   fout.write(tableL +";\n");
           }
                     
           //System.out.println("Number of values : "  + outcome.length);
       }
     }
    String factorStr=networkStr+"Factors";
    String vectStr=" vector<Factor> "+ factorStr+";";
    System.out.println(vectStr);
    fout.write(vectStr);
    for (int temp = 0; temp < nListDef.getLength(); temp++) {
    	System.out.println(factorStr+".push_back( "+factorLineArr[temp]+" );");
    }
    System.out.println("FactorGraph "+networkStr+"Network( "+factorStr+" );");
    System.out.println(networkStr+"Network.WriteToFile( \""+ networkStr+".fg\");" );
    System.out.println("return 0; }");
    fout.write("FactorGraph "+networkStr+"Network( "+factorStr+" );\n");
    fout.write(networkStr+"Network.WriteToFile( \""+ networkStr+".fg\");\n" );
    fout.write("return 0; }\n");
    fout.close();
  } catch (Exception e) {
    e.printStackTrace();
    System.out.println(e);
  }
 }
 
 private static String[] getTagValue(String sTag, Element eElement){
	 NodeList nlList= eElement.getElementsByTagName(sTag);
	 int len=nlList.getLength();
	// System.out.println("length "+len);
	 String[] value=new String[len];
	 for (int temp = 0; temp < len; temp++) { 
		 Node nValue = (Node) nlList.item(temp).getFirstChild();
		 value[temp]=nValue.getNodeValue();
		// System.out.print(value[temp] + " ");
	 }
	// System.out.println();
    return value;    
 }
 private static String getNameValue(String sTag, Element eElement){
	    NodeList nlList= eElement.getElementsByTagName(sTag).item(0).getChildNodes();
	    Node nValue = (Node) nlList.item(0); 
	 
	    return nValue.getNodeValue();    
	 }

}