<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="text" encoding="utf-8" indent="no"/>

    <xsl:template match="thread">
      <!-- TODO add support to count blobs with repeat syntax twice (because we add a loop condition) 
      <xsl:variable name="Size" select="count(//blob|//msg|//sig|//con)"/>
      </xsl:text><xsl:value-of select="$Size"/><xsl:text> -->
      <xsl:call-template name="header"/>
      <xsl:call-template name="data"/>
      <xsl:call-template name="code"/>
      <xsl:call-template name="footer"/>
    </xsl:template>
    
    <!-- Create all Schedule Data Blocks -->
    <xsl:template name="data">
    <!-- TODO Header -->
    <xsl:text>t_data data[] =&#13;&#10;</xsl:text> 
      <xsl:call-template name="datablock"/>
      <xsl:text>;&#13;&#10;</xsl:text>
    <!-- TODO Footer -->  
    </xsl:template>
    
    <!-- Create Schedule Code -->
    <xsl:template name="code">
    <!-- TODO Header -->
    <xsl:text>t_data data[] =&#13;&#10;</xsl:text> 
      <xsl:call-template name="codesnippet"/>
      <xsl:text>;&#13;&#10;</xsl:text>
    <!-- TODO Footer -->   
    </xsl:template>
    
    <!-- Create a Data Block for a Blob, Message, Signal or Condition -->
    <xsl:template name="datablock">
        <xsl:variable name="TagName" select="name()"/>
        <xsl:choose>
            <xsl:when test="$TagName='msg'"><xsl:call-template name="msgdata"/></xsl:when>
            <xsl:when test="$TagName='sig'"><xsl:call-template name="sigdata"/></xsl:when>
            <xsl:when test="$TagName='con'"><xsl:call-template name="condata"/></xsl:when>
            <xsl:when test="$TagName='blob'">
              <xsl:call-template name="blob"/>
              <!-- TODO recurse -->
              <!-- TODO insert loop condition if blob has repeat syntax -->
            </xsl:when>
        </xsl:choose>
     </xsl:template> 
     
     <!-- Create the code for a Blob, Message, Signal or Condition -->
     <xsl:template name="codesnippet">
        <!-- TODO ALLLL!!! --> 
        <xsl:variable name="TagName" select="name()"/>
        <xsl:choose>
            <!-- TODO Enum labels for Msg, Sig & Con based on blob label -->
            
            <xsl:when test="$TagName='msg'"><xsl:call-template name="msgcode"/></xsl:when>
            <xsl:when test="$TagName='sig'"><xsl:call-template name="sigcode"/></xsl:when>
            <xsl:when test="$TagName='con'"><xsl:call-template name="concode"/></xsl:when>
            
            
            <xsl:when test="$TagName='blob'">
              <xsl:call-template name="blobcode"/>
              <!-- TODO recurse -->
              <!-- TODO insert loop condition if blob has repeat syntax -->
            </xsl:when>
        </xsl:choose>
     </xsl:template>
 
 
 
 
           
                <xsl:choose>
                    <xsl:when test="boolean(@Value)"><!-- IF-THEN -->
                    <xsl:if test="@Type='string'"><xsl:text>"</xsl:text></xsl:if>
                    <xsl:value-of select="@Value"/>
                    <xsl:if test="@Type='string'"><xsl:text>"</xsl:text></xsl:if>
                    </xsl:when>
                    <xsl:otherwise><xsl:value-of select="."/></xsl:otherwise><!-- ELSE -->
                </xsl:choose>
            </xsl:when>
            <xsl:when test="$TagName='Sequence'">
                <xsl:text>{ </xsl:text>
                <xsl:for-each select="Element|Sequence|Array">
                    <xsl:call-template name="outputValue"/>
                    <xsl:if test="position() != last()"><xsl:text>,</xsl:text></xsl:if>
                    <xsl:text>&#32;</xsl:text>
                </xsl:for-each>
                <xsl:text>}</xsl:text>
            </xsl:when>
            <xsl:when test="$TagName='Array'">
                <xsl:text>{ </xsl:text>
                <xsl:for-each select="Entry">
                    <xsl:call-template name="outputValue"/>
                    <xsl:if test="position() != last()"><xsl:text>,</xsl:text></xsl:if>
                    <xsl:text>&#32;</xsl:text>
                </xsl:for-each>
                <xsl:text>}</xsl:text>
            </xsl:when>
        </xsl:choose>
    </xsl:template>

    
    
    <xsl:template match="blob">
        <xsl:value-of select="@Name"/>
        <xsl:choose>
            <xsl:when test="'#define'=@Qualifier">
                <!-- The element is output as a macro definition -->
                <xsl:text>#define </xsl:text>
                <xsl:value-of select="@Name"/><xsl:text>&#32;</xsl:text><xsl:value-of select="@Value"/>
            </xsl:when>
            <xsl:otherwise>
                <!-- The element is output as a variable definition -->
                <xsl:call-template name="outputCQualifier"/><!-- Function Call!  -->
                <xsl:call-template name="outputType"/><xsl:text>&#32;</xsl:text>
                <xsl:value-of select="@Name"/><xsl:text> = </xsl:text><xsl:value-of select="@Value"/>
                <xsl:text>;</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:text>&#13;&#10;</xsl:text>
    </xsl:template>

    <xsl:template match="Element">
        <xsl:choose>
            <xsl:when test="'#define'=@Qualifier">
                <!-- The element is output as a macro definition -->
                <xsl:text>#define </xsl:text>
                <xsl:value-of select="@Name"/><xsl:text>&#32;</xsl:text><xsl:value-of select="@Value"/>
            </xsl:when>
            <xsl:otherwise>
                <!-- The element is output as a variable definition -->
                <xsl:call-template name="outputCQualifier"/><!-- Function Call!  -->
                <xsl:call-template name="outputType"/><xsl:text>&#32;</xsl:text>
                <xsl:value-of select="@Name"/><xsl:text> = </xsl:text><xsl:value-of select="@Value"/>
                <xsl:text>;</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:text>&#13;&#10;</xsl:text>
    </xsl:template>

    <xsl:template match="Sequence">
        <xsl:call-template name="outputCQualifier"/>
        <xsl:text>struct </xsl:text><xsl:value-of select="@StructTag"/><xsl:text>&#32;</xsl:text>
        <xsl:value-of select="@Name"/><xsl:text> =&#13;&#10;</xsl:text>
           <xsl:call-template name="outputValue"/>
        <xsl:text>;&#13;&#10;</xsl:text>
    </xsl:template>

    <xsl:template match="Array">
        <xsl:call-template name="outputCQualifier"/>
        <xsl:call-template name="outputType"/><xsl:text>&#32;</xsl:text>
        <xsl:value-of select="@Name"/>
        <xsl:text>[</xsl:text><xsl:value-of select="@Size"/><xsl:text>] =&#13;&#10;</xsl:text>
           <xsl:call-template name="outputValue"/>
        <xsl:text>;&#13;&#10;</xsl:text>
    </xsl:template>

    <xsl:template name="outputCQualifier">
        <xsl:if test="''!=@Qualifier"><!-- IF-THEN only -->
            <xsl:value-of select="$Qualifier"/><xsl:text>&#32;</xsl:text>
        </xsl:if>
    </xsl:template>

    <xsl:template name="outputType">
        <xsl:choose><!-- SWITCH-CASE using cascaded IF's -->
            <xsl:when test="@Type='byte'">unsigned char</xsl:when>
            <xsl:when test="@Type='word'">unsigned short</xsl:when>
            <xsl:when test="@Type='enum'">
                <xsl:text>enum </xsl:text><xsl:value-of select="@EnumTag"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:message terminate="no">
                     <xsl:text>#missing type at node: </xsl:text>
                    <!-- Invoke XSL Code to formulate XPath of current node based on the Schema -->
                </xsl:message>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="outputValue">
        <xsl:variable name="TagName" select="name()"/>
        <xsl:choose>
            <xsl:when test="$TagName='Element' or $TagName='Entry'">
                <xsl:choose>
                    <xsl:when test="boolean(@Value)"><!-- IF-THEN -->
                    <xsl:if test="@Type='string'"><xsl:text>"</xsl:text></xsl:if>
                    <xsl:value-of select="@Value"/>
                    <xsl:if test="@Type='string'"><xsl:text>"</xsl:text></xsl:if>
                    </xsl:when>
                    <xsl:otherwise><xsl:value-of select="."/></xsl:otherwise><!-- ELSE -->
                </xsl:choose>
            </xsl:when>
            <xsl:when test="$TagName='Sequence'">
                <xsl:text>{ </xsl:text>
                <xsl:for-each select="Element|Sequence|Array">
                    <xsl:call-template name="outputValue"/>
                    <xsl:if test="position() != last()"><xsl:text>,</xsl:text></xsl:if>
                    <xsl:text>&#32;</xsl:text>
                </xsl:for-each>
                <xsl:text>}</xsl:text>
            </xsl:when>
            <xsl:when test="$TagName='Array'">
                <xsl:text>{ </xsl:text>
                <xsl:for-each select="Entry">
                    <xsl:call-template name="outputValue"/>
                    <xsl:if test="position() != last()"><xsl:text>,</xsl:text></xsl:if>
                    <xsl:text>&#32;</xsl:text>
                </xsl:for-each>
                <xsl:text>}</xsl:text>
            </xsl:when>
        </xsl:choose>
    </xsl:template>

</xsl:stylesheet>
